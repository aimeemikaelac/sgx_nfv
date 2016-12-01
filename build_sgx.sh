#!/bin/bash

#TODO: add some arg parsing to allow for specifying build dir and what to install

git submodule update
cd linux-sgx/
echo "Building SGX with default options (with default install dir- /opt/intel/sgxsdk)"

echo "Installing dependencies"
sudo apt-get install build-essential ocaml automake autoconf libtool
sudo apt-get install libcurl4-openssl-dev protobuf-compiler protobuf-c-compiler libprotobuf-dev libprotobuf-c0-dev

./download_prebuilt.sh

make

make sdk_install_pkg

make psw_install_pkg

repo_dir=$(pwd)

sudo mkdir -p /opt/intel

cd /opt/intel

sudo su -c "$repo_dir/linux/installer/bin/sgx_linux_x64_sdk_1.6.100.34922.bin"

sudo chmod -R 0777 /opt/intel
