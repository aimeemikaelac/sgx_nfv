#!/bin/bash

SCRIPT=$(realpath $0)
SCRIPTPATH=$(dirname $SCRIPT)
$DIR=$SCRIPTPATH/..
cd $DIR
mkdir -p $DIR/click_out

click_elements=( "basicelement" "basicelementnosgx" )

echo "Make sure to have run build_sgx.sh before running this script"
echo "Otherwise the SGX module won't build"

#make sure click submodule is there
git submodule init
git submodule update

#build SGX module
cd $DIR/NFV_Basic_SGX/sgx/enclave_enclave1
make clean
make SGX_MODE=HW SGX_PRERELEASE=1 lib

cd $DIR/click/elements/local
for i in "${click_elements[@]}"
do
  ln -s $DIR/click_experiments/$i.cc
  ln -s $DIR/click_experiments/$i.hh
done


cd $DIR/click
#get rid of a previous installation
make uninstall
make clean
#set environmental variables for SGX simulation
export CPPFLAGS="-I/opt/intel/sgxsdk/include/ -I$DIR/NFV_Basic_SGX/sgx/enclave_enclave1/untrusted/"
export LDFLAGS="-L$DIR/NFV_Basic_SGX/sgx/enclave_enclave1/ -L/opt/intel/sgxsdk/lib64"
export LIBS="-lapp -lsgx_urts -lsgx_uae_service -lpthread -lcrypto"
#build
./configure --enable-local --prefix=$DIR/click_out
make
make install
