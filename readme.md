# cpp-toolbox —— 可拷贝的 C++ 依赖包（`.h` + `.a`）

目标：把自研工具和第三方库整理成一个 `dist/` 目录，**像 Python 的 import 一样**在别的项目里直接使用——
把 `dist/` 整个拷过去即可，不需要 conan、不需要 cmake，只用 `g++`/`ar`。

## 目录结构

```
include/mytools/     自研库头文件（lru / crypto / lda / blockchain）
src/lib/             自研库实现（编成 libmytools.a）
src/                 各种 demo（test_*.cpp、tools/、alg/）
examples/            下游使用示例
scripts/
  build_lib.sh       纯 g++ + ar 编自研库
  export_deps.sh     用 conan 一次性导出第三方依赖（含 cmake/ninja 的隔离环境）
  export_dist.py     把第三方 .h/.a 合并进 dist/ 并改写 pkg-config（用 ${pcfiledir} 可搬迁）
dist/                最终产物（可整个拷走）
  include/           所有头文件（boost/absl/grpcpp/...）
  lib/               libmytools.a 及 170+ 个第三方 .a
  bin/               protoc 27.0 / grpc_*_plugin / openssl 等工具
  pkgconfig/         改写过的 .pc，可直接 pkg-config
  env.sh             source 后自动设好路径（用脚本自身位置定位，可搬迁）
```

## 一、构建自研库（最快，不需要 conan/cmake）

```bash
./build.sh            # 等价于 scripts/build_lib.sh
```

只用 `g++` + `ar`，几秒钟得到 `dist/lib/libmytools.a` + `dist/include/mytools/`。

## 二、导出第三方依赖（需要 conan）

```bash
./build.sh deps
```

`scripts/export_deps.sh` 会：
1. 用仓库内 `.venv/bin/conan`（没有则用 PATH 里的 conan）`conan install`，
   把依赖编译好（静态 `shared=False`，spdlog 用 header-only + `std::format`）；
2. 用 `scripts/export_dist.py` 把每个包的 `include/`、`lib/*.a`、`bin/` 合并进 `dist/`，
   并把 `.pc` 的 `prefix` 改写成 `${pcfiledir}/..`。

> - 首次会从源码编译 `boost / grpc / abseil / protobuf / openssl`，耗时较长（本机约 40–60 分钟）。
> - conan 会自带 cmake/autotools/b2 等构建工具，系统无需预装。
> - 下游用 spdlog 时请用 `-std=c++20`（`std::format` 要求）。
> - `dist/` 约 450MB（大头是 boost 头文件），可整目录拷走。

## 二·五、按需导出单个包（推荐，避免 450MB 全量）

如果只需要其中几个库，用 `export_pkg.sh` 只装指定包（含传递依赖），生成精简 `dist/`：

```bash
./scripts/export_pkg.sh spdlog                 # 版本自动从 conanfile.txt 读
./scripts/export_pkg.sh grpc/1.67.1            # 也可直接写 name/version
./scripts/export_pkg.sh spdlog rapidjson zlib  # 一次多个
./scripts/export_pkg.sh --list                 # 列出可选包
./scripts/export_pkg.sh --no-reset zlib        # 追加进现有 dist，不清空
./scripts/export_pkg.sh --dist /tmp/d zlib     # 输出到别的目录
```

默认会清空重建 `dist/`，并自动补上自研库 `libmytools.a`。例：只导 spdlog 时，
`dist/` 里就只有 spdlog 头文件 + mytools，体积从 450MB 降到几 MB。

## 三、在别的项目里使用

把生成的 `dist/` 拷到任何地方，然后：

```bash
source /path/to/dist/env.sh        # 设置 MYTOOLS_DIST / PKG_CONFIG_PATH / PATH
```

之后像 `import` 一样用（`dist` 里都是静态库，统一加 `--static` 带出全部传递依赖）：

```bash
g++ -std=c++20 main.cpp $(pkg-config --cflags --libs --static mytools)   -o main
g++ -std=c++20 main.cpp $(pkg-config --cflags --libs --static spdlog)    -o main
g++ -std=c++20 main.cpp $(pkg-config --cflags --libs --static grpc++ protobuf) -o main
g++ -std=c++20 main.cpp $(pkg-config --cflags --libs --static redis++ libuv)   -o main
```

完整示例见 `examples/use_mytools.cpp`：

```cpp
#include "mytools/lru.hpp"
#include "mytools/crypto.hpp"

mytools::LruCache<std::string, int> cache(2);
cache.put("a", 1);
std::cout << mytools::CryptoToolbox::MD5Hash("hello") << "\n";
```

## 自研库提供的内容

| 头文件 | 说明 | 链接 |
| --- | --- | --- |
| `mytools/lru.hpp` | 通用 LRU 缓存（模板） | header-only |
| `mytools/crypto.hpp` | AES/DES/RSA/哈希/Base64/凯撒/希尔 | `-lmytools` + openssl/zlib |
| `mytools/lda.hpp` | LDA 主题模型 + 配置/语料读取 | `-lmytools` |
| `mytools/blockchain.hpp` | 简易区块链 | `-lmytools` + openssl |

`pkg-config --cflags --libs --static mytools` 会自动带上 `-lcrypto -lz -ldl -lpthread`。

## 常用命令

```bash
make -C src                 # 用 dist/ 编译全部 demo
make -C src tools           # LDA / CryptoToolbox / blockChain
make -C src server client   # grpc 示例（先 make -C src proto）
make -C src pred            # K线预测（abseil）
make -C src clean
```

## 备注

- `bin/` 里提交了 protoc / grpc_*_plugin；导出依赖后 `dist/bin/` 会有 protobuf 5.27 自带的
  `protoc` 27.0 及全部 grpc 插件，可随 dist 一起分发。
- `dist/`、`build/`、`.venv/` 都不提交 git。
- 本机（Debian 13 / gcc 14.2）已实测：spdlog、rapidjson、libnpy、abseil、redis++、
  grpc(server/client)、LDA、Crypto、blockChain、pred、mytools 全部可用，
  且 `dist/` 搬迁到新路径后仍能编译运行。
- rapidjson 用 `cci.20250205`（1.1.0 与 gcc14 不兼容）。
