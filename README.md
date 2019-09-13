# wasmassp

webasembly combilable version of the libassp

## compile:

- make sure emsdk is activated: `source ../emsdk/emsdk_env.sh` (change path to ur install)

- compile `emcc -O3 -s WASM=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' \
    -I assp \
    performAssp.c \
    assp/*.c \
    --preload-file resources/msajc003.wav \
    --emrun`

- view `emrun --no_browser --port 8080 test.html`