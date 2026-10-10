#!/usr/bin/env bash
# 一键构建。默认只编自研库（纯 g++/ar，秒级完成，不需要 conan/cmake）。
#
#   ./build.sh              # 编自研库 libmytools.a -> dist/
#   ./build.sh deps         # 额外用 conan 导出第三方依赖到 dist/（较慢）
#
# 产物都在 dist/，可直接整个目录拷到别的项目使用（见 readme.md）。
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ "${1:-}" = "deps" ]; then
  bash "$ROOT/scripts/export_deps.sh"
fi

bash "$ROOT/scripts/build_lib.sh"

echo
echo "dist/ 已就绪。下游使用： source $ROOT/dist/env.sh"
