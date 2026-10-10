#!/usr/bin/env bash
# 用 conan 一次性把第三方依赖编译好，再导出成可拷贝的 dist/（下游零 conan）。
# 需要机器上装有 conan，且能编译这些库（cmake/autotools 等）。
# 用法:
#   CONAN_BIN=/path/to/conan ./scripts/export_deps.sh
#   ./scripts/export_deps.sh            # 默认用 PATH 里的 conan
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# 优先用仓库内自带的 conan 环境（含 cmake/ninja），否则用 PATH 里的
if [ -z "${CONAN_BIN:-}" ]; then
  if [ -x "$ROOT/.venv/bin/conan" ]; then
    CONAN_BIN="$ROOT/.venv/bin/conan"
    export PATH="$ROOT/.venv/bin:$PATH"
  else
    CONAN_BIN="conan"
  fi
fi
OUT="$ROOT/build/conan"

if ! command -v "$CONAN_BIN" >/dev/null 2>&1; then
  echo "找不到 conan: $CONAN_BIN （可用 CONAN_BIN= 指定路径）" >&2
  exit 1
fi

mkdir -p "$OUT"
echo "[export_deps] conan install（首次较慢，会编译 boost/grpc/abseil 等）"
# cppstd=20 是 spdlog(std::format) 和现代 grpc/abseil 需要的；下游编译也请用 -std=c++20
"$CONAN_BIN" install "$ROOT" \
  --build=missing \
  -s compiler.cppstd=20 \
  --output-folder="$OUT"

echo "[export_deps] 导出到头文件/静态库目录 dist/"
python3 "$ROOT/scripts/export_dist.py" --pc-dir "$OUT" --dist "$ROOT/dist" --reset

echo "[export_deps] 完成。下游用法： source dist/env.sh"
