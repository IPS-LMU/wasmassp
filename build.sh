#!/bin/bash

source ../emsdk/emsdk_env.sh

emcc -O3 -s WASM=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' \
    -I assp \
    assp/*.c \
    performAssp.c \
    cJSON/build/local_install_dir/usr/local/lib/libcjson.a \
    -I cJSON/build/local_install_dir/usr/local/include \
    --emrun