SHELL := /bin/bash

# 所有依赖都来自仓库根的 dist/（可用 make dist 生成）
ROOT       := $(abspath $(CURDIR)/..)
DIST       := $(ROOT)/dist
export PKG_CONFIG_PATH := $(DIST)/pkgconfig

CXX      := g++
CXXSTD   ?= -std=c++20
CXXFLAGS := $(CXXSTD) -O2 -fPIC -Wall -I. -I$(DIST)/include
LDFLAGS  := -L$(DIST)/lib

# 自研库（dist/lib/libmytools.a），依赖 openssl，用 pkg-config 带出传递依赖
MYTOOLS := $(shell $(PKG_CONFIG) --cflags --libs --static mytools 2>/dev/null)
ifeq ($(strip $(MYTOOLS)),)
MYTOOLS := -L$(DIST)/lib -lmytools -lcrypto -lz -ldl -lpthread
endif

PKG_CONFIG ?= pkg-config
# dist 里都是静态库，用 --static 才能带出全部传递依赖
GRPC_DEPS  := $(shell $(PKG_CONFIG) --cflags --libs --static grpc++ protobuf 2>/dev/null)
ABSL_DEPS  := $(shell $(PKG_CONFIG) --cflags --libs --static abseil 2>/dev/null)
REDIS_DEPS := $(shell $(PKG_CONFIG) --cflags --libs --static libuv-static redis++ 2>/dev/null)

.PHONY: all deps mytools proto server client \
        test_spdlog test_rapidjson test_redis test_abseil test_pybind11 test_libnpy \
        lda pred crypto blockChain tools clean

all: mytools test_spdlog test_rapidjson test_libnpy

deps:
	$(ROOT)/build.sh deps

mytools:
	$(ROOT)/scripts/build_lib.sh

tools: mytools lda crypto blockChain

lda: mytools
	$(CXX) $(CXXFLAGS) tools/LDA.cpp $(LDFLAGS) $(MYTOOLS) -o LDA.bin

crypto: mytools
	$(CXX) $(CXXFLAGS) tools/CryptoToolbox.cpp $(LDFLAGS) $(MYTOOLS) -o CryptoToolbox.bin

blockChain: mytools
	$(CXX) $(CXXFLAGS) tools/blockChain.cpp $(LDFLAGS) $(MYTOOLS) -o blockChain.bin

pred:
	$(CXX) $(CXXFLAGS) tools/pred.cpp -Wl,--start-group $(ABSL_DEPS) -Wl,--end-group -o pred.bin

test_spdlog:
	$(CXX) $(CXXFLAGS) -DSPDLOG_USE_STD_FORMAT test_spdlog.cpp -o test_spdlog.bin -lpthread

test_rapidjson:
	$(CXX) $(CXXFLAGS) test_rapidjson.cpp -o test_rapidjson.bin

test_libnpy:
	$(CXX) $(CXXFLAGS) test_libnpy.cpp -o test_libnpy.bin

test_abseil:
	$(CXX) $(CXXFLAGS) test_abseil.cpp -Wl,--start-group $(ABSL_DEPS) -Wl,--end-group -o test_absl.bin

test_redis:
	$(CXX) $(CXXFLAGS) test_redis++.cpp -DREDIS_PLUS_PLUS_BUILD_ASYNC=libuv \
		$(REDIS_DEPS) -o test_redis++.bin

test_pybind11:
	$(CXX) $(CXXSTD) -O3 -Wall -shared -fPIC -pthread test_pybind11.cpp -o test_pybind11.so \
		`python3 -m pybind11 --includes`

proto:
	$(ROOT)/bin/protoc --cpp_out=. ./proto/helloworld.proto
	$(ROOT)/bin/protoc --grpc_out=. --plugin=protoc-gen-grpc=$(ROOT)/bin/grpc_cpp_plugin ./proto/helloworld.proto

server: proto
	$(CXX) $(CXXFLAGS) server.cpp ./proto/helloworld.pb.cc ./proto/helloworld.grpc.pb.cc \
		-Wl,--start-group $(GRPC_DEPS) -Wl,--end-group -o server.bin

client: proto
	$(CXX) $(CXXFLAGS) client.cpp ./proto/helloworld.pb.cc ./proto/helloworld.grpc.pb.cc \
		-Wl,--start-group $(GRPC_DEPS) -Wl,--end-group -o client.bin

clean:
	rm -rf *.bin *.o *.so proto/*.pb.cc proto/*.pb.h
