#!/bin/bash

SCRIPT=$(realpath $0)
SCRIPTPATH=$(dirname $SCRIPT)
DIR=$SCRIPTPATH/..
cd $DIR
mkdir -p $DIR/click_out

echo "Make sure to have run build_sgx.sh before running this script"
echo "Otherwise the SGX module won't build"

#make sure click submodule is there
git submodule init
git submodule update

cd $DIR/click
#get rid of a previous installation
make uninstall
make clean
#build
./configure --prefix=$DIR/click_out
make
make install
