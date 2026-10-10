#!/usr/bin/env bash
# 用 g++ + ar 把自研工具编成可拷贝的 .h + .a，输出到 dist/
# 不依赖 cmake / conan，任何有 g++ 的机器都能跑。
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/src/lib"
INC="$ROOT/include"
DIST="${DIST:-$ROOT/dist}"
OBJ="$ROOT/build/obj"

CXX="${CXX:-g++}"
CXXFLAGS="${CXXFLAGS:--std=c++17 -O2 -fPIC -Wall -Wno-deprecated-declarations}"
AR="${AR:-ar}"

mkdir -p "$DIST/include" "$DIST/lib" "$DIST/pkgconfig" "$OBJ"

echo "[1/3] 编译自研库源码 ($SRC)"
for src in "$SRC"/*.cpp; do
  [ -e "$src" ] || continue
  obj="$OBJ/$(basename "${src%.cpp}").o"
  echo "  - $(basename "$src")"
  "$CXX" $CXXFLAGS -I"$INC" -c "$src" -o "$obj"
done

echo "[2/3] 打包 dist/lib/libmytools.a"
"$AR" rcs "$DIST/lib/libmytools.a" "$OBJ"/*.o

echo "[3/3] 拷贝头文件到 dist/include/mytools"
rm -rf "$DIST/include/mytools"
cp -r "$INC/mytools" "$DIST/include/mytools"

echo "       生成 pkg-config: dist/pkgconfig/mytools.pc"
cat > "$DIST/pkgconfig/mytools.pc" <<'EOF'
prefix=${pcfiledir}/..
includedir=${prefix}/include
libdir=${prefix}/lib

Name: mytools
Description: 自研工具库 (lru / crypto / lda / blockchain)
Version: 1.0.0
Cflags: -I${includedir}
Libs: -L${libdir} -lmytools
Libs.private: -lcrypto -lz -ldl -lpthread
EOF

echo "完成："
echo "  headers: $DIST/include/mytools"
echo "  library: $DIST/lib/libmytools.a"
