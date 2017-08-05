#!/bin/bash
set -e

export BINUTILS_VERSION=2.26
export GCC_VERSION=6.1.0
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf

if [ ! -f binutils-$BINUTILS_VERSION.tar.gz ]; then
    wget http://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.gz
    tar xvfz binutils-$BINUTILS_VERSION.tar.gz
fi

if [ ! -f gcc-$GCC_VERSION.tar.gz ]; then
    wget ftp://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz
    tar xvfz gcc-$GCC_VERSION.tar.gz

fi

brew install gmp mpfr libmpc

mkdir build-binutils && pushd build-binutils
    ../binutils-$BINUTILS_VERSION/configure --prefix=$PREFIX \
                                            --target=$TARGET \
                                            --enable-interwork \
                                            --enable-multilib \
                                            --disable-nls \
                                            --disable-werror
    make && make install
popd

mkdir build-gcc && pushd build-gcc
    ../gcc-$GCC_VERSION/configure --prefix=$PREFIX \
                                  --target=$TARGET \
                                  --disable-nls \
                                  --enable-languages=c \
                                  --without-headers \
                                  --enable-interwork \
                                  --enable-multilib \
                                  --with-gmp=/usr \
                                  --with-mpc=/opt/local \
                                  --with-mpfr=/opt/local
    make all-gcc all-target-libgcc
    make install-gcc
    make install-target-libgcc
popd
