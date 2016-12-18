#!/bin/bash

SCRIPT=$(realpath $0)
SCRIPTPATH=$(dirname $SCRIPT)

git submodule init
git submodule update

mkdir -p openssl_out
cd $SCRIPTPATH/openssl
export CFLAGS=-fPIC
make uninstall
make clean
CFLAGS=-fPIC -static -static-libgcc ./config -fPIC -static -static-libgcc  --prefix=$SCRIPTPATH/openssl_out --openssldir=$SCRIPTPATH/openssl_out
make
make install
