#!/bin/sh
pushd build
bakefile_gen
./acregen.sh
popd
./configure
