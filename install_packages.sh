#!/bin/bash

#steps (install below packages locally)
#1. install pod5
#2. install slow5lib
#3. install boost
#4. install zlib
#6. install cuda
#7. install cudDNN
#8. make

# set -x
RED='\033[0;31m' ; GREEN='\033[0;32m' ; NC='\033[0m' # No Color
die() { echo -e "${RED}$1${NC}" >&2 ; echo ; exit 1 ; } # terminate script
info() {  echo ; echo -e "${GREEN}$1${NC}" >&2 ; }
info "$(date)"

info "boost_1_86_0"
wget https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.gz
tar -xvf boost_1_86_0.tar.gz
cd boost_1_86_0
./bootstrap.sh
./b2 --with-random
mv boost_1_86_0 boost
rm boost_1_86_0.tar.gz

info "pod5-0.3.23"
cd pod5-file-format
wget https://github.com/nanoporetech/pod5-file-format/releases/download/0.3.23/lib_pod5-0.3.23-linux-x64.tar.gz
tar -xvf lib_pod5-0.3.23-linux-x64.tar.gz
rm lib_pod5-0.3.23-linux-x64.tar.gz
cp pod5_format_export.h include/pod5_format
cd ..

info "slow5lib-1.3.0"
wget https://github.com/hasindu2008/slow5lib/archive/refs/tags/v1.3.0.tar.gz
tar -xvf v1.3.0.tar.gz
rm v1.3.0.tar.gz
mv slow5lib-1.3.0 slow5lib

info "zlib-1.2.13"
wget https://github.com/madler/zlib/releases/download/v1.2.13/zlib-1.2.13.tar.gz
tar -xvzf zlib-1.2.13.tar.gz
cd zlib-1.2.13
./configure --prefix=$PWD
make
make install
rm zlib-1.2.13.tar.gz
mv zlib-1.2.13 zlib

info "cuda_11.0.2"
wget "http://developer.download.nvidia.com/compute/cuda/11.0.2/local_installers/cuda_11.0.2_450.51.05_linux.run"
sh ./cuda_11.0.2_450.51.05_linux.run --silent --toolkit --toolkitpath=$PWD/cuda-11.0 --override
rm cuda_11.0.2_450.51.05_linux.run

info "cudnn-11.0-linux-x64-v8.0.2.39"
info "go to https://developer.nvidia.com/rdp/cudnn-archive and select Download cuDNN v8.0.2 (July 24th, 2020), for CUDA 11.0 and download cuDNN Library for Linux (x86_64)"
tar -xvf cudnn-11.0-linux-x64-v8.0.2.39.tgz || die "cudnn-11.0-linux-x64-v8.0.2.39.tgz not found. donwload it manually"
rm cudnn-11.0-linux-x64-v8.0.2.39.tgz
mv cuda cudnn-v8.0-cuda-11.0

info "all packages are installed"
info "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/pod5-file-format/lib:$PWD/zlib/lib:$PWD/cuda-11.0/lib64:$PWD/cudnn-v8.0-cuda-11.0/lib64"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/pod5-file-format/lib:$PWD/zlib/lib:$PWD/cuda-11.0/lib64:$PWD/cudnn-v8.0-cuda-11.0/lib64
make clean && make zstd=1 -j || die "make failed"
./bin/DNAscent --version || die "DNAscent failed"