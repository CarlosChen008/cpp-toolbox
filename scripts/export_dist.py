#!/usr/bin/env python3
"""把 conan(PkgConfigDeps) 生成的 .pc 依赖收集合并成可拷贝的 dist/。

用法:
    python3 scripts/export_dist.py --pc-dir <生成.pc的目录> --dist dist

产物:
    dist/include/       所有第三方头文件（合并）
    dist/lib/           所有 .a / .so* 静态或动态库（合并）
    dist/bin/           protoc 等工具（若有）
    dist/pkgconfig/     改写 prefix 后的 .pc，可直接 pkg-config 使用
    dist/env.sh         source 后设置 PKG_CONFIG_PATH / PATH / CPLUS_INCLUDE_PATH
"""
import argparse
import re
import shutil
import subprocess
import sys
from pathlib import Path

LIB_SUFFIXES = (".a", ".so", ".dylib", ".lib")


def expand(value: str, variables: dict, depth: int = 0) -> str:
    if depth > 10 or value is None:
        return value

    def repl(m):
        name = m.group(1)
        return variables.get(name, m.group(0))

    return expand(re.sub(r"\$\{(\w+)\}", repl, value), variables, depth + 1)


def parse_pc(path: Path) -> dict:
    text = path.read_text(errors="replace")
    variables = {}
    for line in text.splitlines():
        m = re.match(r"^\s*([A-Za-z_]\w*)\s*=\s*(.*)$", line)
        if not m:
            continue
        key, val = m.group(1), m.group(2).strip()
        if key in ("Name", "Description", "Version", "Requires", "Libs", "Cflags"):
            continue
        variables[key] = val
    for _ in range(10):
        variables = {k: expand(v, variables) for k, v in variables.items()}

    prefix = variables.get("prefix", "")
    includedir = variables.get("includedir", str(Path(prefix) / "include"))
    libdir = variables.get("libdir", str(Path(prefix) / "lib"))
    return {
        "raw": text,
        "variables": variables,
        "prefix": prefix,
        "includedir": includedir,
        "libdir": libdir,
        "text": text,
    }


def copy_tree(src: Path, dst: Path) -> int:
    """把 src 下的内容合并进 dst（用 cp -a，比 Python 逐文件快很多）。"""
    if not src.is_dir():
        return 0
    dst.mkdir(parents=True, exist_ok=True)
    subprocess.run(["cp", "-a", f"{src}/.", str(dst)], check=False)
    return 1


def copy_libs(src: Path, dst: Path) -> int:
    if not src.is_dir():
        return 0
    libs = [
        f
        for f in src.iterdir()
        if f.is_file() and (f.name.endswith(LIB_SUFFIXES) or ".so." in f.name)
    ]
    if not libs:
        return 0
    subprocess.run(["cp", "-a", *[str(f) for f in libs], str(dst)], check=False)
    return len(libs)


def rewrite_pc(info: dict, dist: Path, replacements: dict) -> str:
    text = info["text"]
    # 1) prefix 用 ${pcfiledir}/.. 动态定位，保证 dist 整个目录被拷走后仍可用
    text = re.sub(r"^prefix\s*=.*$", "prefix=${pcfiledir}/..", text, flags=re.M)
    text = re.sub(r"^includedir\s*=.*$", "includedir=${prefix}/include", text, flags=re.M)
    text = re.sub(r"^libdir\s*=.*$", "libdir=${prefix}/lib", text, flags=re.M)
    # 2) 清掉直接写死的 conan 缓存绝对路径：路径长的先替换，避免前缀被先吃掉
    for path in sorted(replacements, key=len, reverse=True):
        if path:
            text = text.replace(path, replacements[path])
    return text


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--pc-dir", required=True, help="包含 .pc 文件的目录")
    ap.add_argument("--dist", default="dist")
    ap.add_argument("--reset", action="store_true", help="导出前清空 dist 的 include/lib/bin/pkgconfig")
    args = ap.parse_args()

    pc_dir = Path(args.pc_dir).resolve()
    dist = Path(args.dist).resolve()
    if not pc_dir.is_dir():
        print(f"[export] 找不到 .pc 目录: {pc_dir}", file=sys.stderr)
        return 1

    pc_files = sorted(pc_dir.glob("*.pc"))
    if not pc_files:
        print(f"[export] {pc_dir} 下没有 .pc 文件", file=sys.stderr)
        return 1

    if args.reset:
        for sub in ("include", "lib", "bin", "pkgconfig"):
            shutil.rmtree(dist / sub, ignore_errors=True)
    (dist / "include").mkdir(parents=True, exist_ok=True)
    (dist / "lib").mkdir(parents=True, exist_ok=True)
    (dist / "pkgconfig").mkdir(parents=True, exist_ok=True)

    infos = [(pc, parse_pc(pc)) for pc in pc_files]

    replacements = {}
    for _pc, info in infos:
        if info["includedir"]:
            replacements[info["includedir"]] = "${prefix}/include"
        if info["libdir"]:
            replacements[info["libdir"]] = "${prefix}/lib"
        if info["prefix"]:
            replacements[info["prefix"]] = "${prefix}"

    # 同一个 include/lib/bin 目录只拷一次（abseil 等有上百个组件 .pc 共享目录）
    done_dirs = set()
    n_headers = n_libs = 0
    for pc, info in infos:
        inc = info["includedir"]
        if inc and inc not in done_dirs:
            done_dirs.add(inc)
            n_headers += copy_tree(Path(inc), dist / "include")
        lib = info["libdir"]
        if lib and lib not in done_dirs:
            done_dirs.add(lib)
            n_libs += copy_libs(Path(lib), dist / "lib")
        bindir = str(Path(info["prefix"]) / "bin")
        if bindir not in done_dirs and Path(bindir).is_dir():
            done_dirs.add(bindir)
            (dist / "bin").mkdir(exist_ok=True)
            copy_tree(Path(bindir), dist / "bin")
        (dist / "pkgconfig" / pc.name).write_text(rewrite_pc(info, dist, replacements))

    env = dist / "env.sh"
    env.write_text(
        "#!/usr/bin/env bash\n"
        "# source 本文件即可让下游项目在没有 conan 的情况下使用 dist/\n"
        "# 用脚本自身位置动态定位，dist/ 整个目录拷到任何地方都能用\n"
        'export MYTOOLS_DIST="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"\n'
        'export PKG_CONFIG_PATH="$MYTOOLS_DIST/pkgconfig:${PKG_CONFIG_PATH:-}"\n'
        'export CPLUS_INCLUDE_PATH="$MYTOOLS_DIST/include:${CPLUS_INCLUDE_PATH:-}"\n'
        'export LIBRARY_PATH="$MYTOOLS_DIST/lib:${LIBRARY_PATH:-}"\n'
        'export LD_LIBRARY_PATH="$MYTOOLS_DIST/lib:${LD_LIBRARY_PATH:-}"\n'
        'export PATH="$MYTOOLS_DIST/bin:${PATH:-}"\n'
    )
    env.chmod(0o755)

    print(f"[export] 合并 {len(pc_files)} 个包: {n_headers} 个头文件, {n_libs} 个库文件")
    print(f"[export] dist 根目录: {dist}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
