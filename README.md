# wasmassp

webasembly compilable version of the libassp

## compile:

- install emsdk
- install dependencies: `./install_deps.sh` (needs `cmake`)
- make sure correct emsdk is activated: `source ../emsdk/emsdk_env.sh` line in `build.sh` (change path to ur install)
- build/compile `./build.sh`
- view `emrun --no_browser --port 8080 test.html` (only console output 4 now)