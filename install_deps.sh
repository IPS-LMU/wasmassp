#!/bin/bash

# install cJSON

git clone https://github.com/DaveGamble/cJSON.git
cd cJSON
mkdir build                                      
cd build
cmake ..
make 
make DESTDIR=temp install