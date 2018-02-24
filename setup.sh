#!/bin/bash

export BINUTILS_VERSION=2.26
export GCC_VERSION=6.1.0
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf

if [ -f $HOME/opt/cross/bin/i67-elf-gcc ]; then
    wget http://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz
    tar xvfz binutils-$BINUTILS_VERSION.tar.gz

    wget ftp://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz
    tar xvfz gcc-$GCC_VERSION.tar.gz

    if [[ `uname` == 'Darwin' ]]; then
        brew install gmp mpfr libmpc
    elif [[ `uname` == 'Linux' ]]; then
        apt-get install libmpc-dev xorriso qemu-system-x86 libc6-dev-i386
    fi

    mkdir -p build-binutils && pushd build-binutils
        ../binutils-$BINUTILS_VERSION/configure --prefix=$PREFIX \
                                                --target=$TARGET \
                                                --disable-nls \
                                                --with-sysroot \
                                                --disable-werror
        make && make install
    popd

    # Build a cross-compiler that links against newbos.
    if [ ! -f build-gcc ]; then
        mkdir -p build-gcc && pushd build-gcc
            ../gcc-$GCC_VERSION/configure --prefix=$PREFIX \
                                          --target=$TARGET \
                                          --disable-nls \
                                          --enable-languages=c \
                                          --without-headers
            make all-gcc all-target-libgcc
            make install-gcc
            make install-target-libgcc
        popd
    fi
fi

export CC=$HOME/opt/cross/bin/i686-elf-gcc
export AR=$HOME/opt/cross/bin/i686-elf-ar
export AS=$HOME/opt/cross/bin/i686-elf-as
