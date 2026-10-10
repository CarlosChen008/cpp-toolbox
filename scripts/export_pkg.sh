#!/usr/bin/env bash
# 按需导出：只安装指定的包（及其传递依赖），生成精简的 dist/。
#
#   ./scripts/export_pkg.sh spdlog              # 版本从 conanfile.txt 读
#   ./scripts/export_pkg.sh grpc/1.67.1         # 也可直接写 name/version
#   ./scripts/export_pkg.sh spdlog rapidjson    # 一次多个
#   ./scripts/export_pkg.sh --list              # 列出 conanfile.txt 里可选的包
#   ./scripts/export_pkg.sh --no-reset zlib     # 追加进现有 dist，而不是清空重建
#   ./scripts/export_pkg.sh --dist /tmp/d rapidjson   # 输出到别的目录
#
# 默认会清空并重建 dist/（再补上自研库 libmytools.a）。
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PKGS=()
DIST="$ROOT/dist"
RESET=1

usage() {
  sed -n '2,12p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
  exit "${1:-1}"
}

while [ $# -gt 0 ]; do
  case "$1" in
    --list|-l)
      echo "conanfile.txt 里可用的包："
      awk '/^\[requires\]/{f=1;next} /^\[/{f=0} f' "$ROOT/conanfile.txt" | grep -E '^[A-Za-z]' | sed 's/^/  /'
      exit 0 ;;
    --no-reset) RESET=0 ;;
    --dist) DIST="$2"; shift ;;
    -h|--help) usage 0 ;;
    *) PKGS+=("$1") ;;
  esac
  shift
done

if [ ${#PKGS[@]} -eq 0 ]; then
  echo "错误：没有指定包" >&2
  usage
fi

# 优先用仓库内自带的 conan 环境（含 cmake/ninja）
if [ -x "$ROOT/.venv/bin/conan" ]; then
  CONAN_BIN="$ROOT/.venv/bin/conan"
  export PATH="$ROOT/.venv/bin:$PATH"
else
  CONAN_BIN="${CONAN_BIN:-conan}"
fi
command -v "$CONAN_BIN" >/dev/null 2>&1 || { echo "找不到 conan: $CONAN_BIN" >&2; exit 1; }

# conanfile.txt 里的 [requires]
mapfile -t AVAIL < <(awk '/^\[requires\]/{f=1;next} /^\[/{f=0} f' "$ROOT/conanfile.txt" | grep -E '^[A-Za-z]')

# 把用户输入解析成 name/version
REFS=()
for pkg in "${PKGS[@]}"; do
  if [[ "$pkg" == */* ]]; then
    REFS+=("$pkg")
    continue
  fi
  ref=""
  for a in "${AVAIL[@]}"; do
    if [[ "$a" == "$pkg/"* ]]; then ref="$a"; break; fi
  done
  if [ -z "$ref" ]; then
    echo "错误：conanfile.txt 里没有 '$pkg'。可选：" >&2
    printf '  %s\n' "${AVAIL[@]}" >&2
    exit 1
  fi
  REFS+=("$ref")
done

# 生成临时 conanfile：只含选中的包，复用原文件的 [generators] 和 [options]
OUT="$ROOT/build/pkgconan"
TMP="$ROOT/build/pkg_export_conanfile.txt"
mkdir -p "$OUT"
{
  echo "[requires]"
  printf '%s\n' "${REFS[@]}"
  echo
  awk '/^\[generators\]/{f=1} f' "$ROOT/conanfile.txt"
} > "$TMP"

echo "[export_pkg] 安装: ${REFS[*]}"
"$CONAN_BIN" install "$TMP" \
  --build=missing \
  -s compiler.cppstd=20 \
  --output-folder="$OUT"

EXTRA=()
[ "$RESET" -eq 1 ] && EXTRA+=("--reset")
echo "[export_pkg] 导出到 $DIST"
python3 "$ROOT/scripts/export_dist.py" --pc-dir "$OUT" --dist "$DIST" "${EXTRA[@]}"

# dist 被清空后重新补上自研库
if [ "$RESET" -eq 1 ]; then
  DIST="$DIST" bash "$ROOT/scripts/build_lib.sh" >/dev/null
fi

echo "[export_pkg] 完成。用法： source $DIST/env.sh"
