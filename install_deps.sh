#!/bin/bash

npm install

# activate emsdk
source ../emsdk/emsdk_env.sh

# install cJSON

git clone https://github.com/DaveGamble/cJSON.git
cd cJSON
mkdir build                                      
cd build
emconfigure cmake ..
emmake make
make DESTDIR=local_install_dir install
cd .. # not needed in script just 4 copy & paste help
cd .. # same
