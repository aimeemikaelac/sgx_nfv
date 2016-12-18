#!/bin/bash

SCRIPT=$(realpath $0)
SCRIPTPATH=$(dirname $SCRIPT)

git submodule init
git submodule update

rm -rf libtomcrypt_out

mkdir -p libtomcrypt_out/lib
mkdir -p libtomcrypt_out/include
mkdir -p libtomcrypt_out/docs

cd libtomcrypt
alias latex=""
make clean
make DESTDIR=$SCRIPTPATH/libtomcrypt_out LIBPATH=/lib INCPATH=/include INSTALL_GROUP=michael USER=michael NODOCS=true install
