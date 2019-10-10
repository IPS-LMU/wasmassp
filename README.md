# wasmassp

webasembly compilable version of the libassp

## compile:

- install emsdk (see https://emscripten.org/docs/getting_started/downloads.html)
- make sure correct emsdk is activated: `source ../emsdk/emsdk_env.sh` line in `build.sh` and `install_deps.sh` (change path to ur install)
- install dependencies: `./install_deps.sh` (needs `cmake`)
- build/compile `./build.sh`
- run test webpage `emrun --no_browser --port 8080 test.html`
- go to http://localhost:8080/test.html