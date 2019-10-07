#!/bin/bash

source ../emsdk/emsdk_env.sh

emcc -O3 -s WASM=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' \
    -I assp \
    -I cJSON/build/temp/usr/local/include \
    -L cJSON/build/temp/usr/local/lib \
    performAssp.c \
    assp/*.c \
    --emrun