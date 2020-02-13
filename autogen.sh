#!/bin/bash

MY_PREFIX=`pwd`/lib/usr
if [ ! "${PREFIX}" = "" ]; then
MY_PREFIX="${PREFIX}"
fi
echo "=========== libosporting MY_PREFIX=${MY_PREFIX}"

./autoclean.sh

rm -f configure

rm -f Makefile.in

rm -f config.guess
rm -f config.sub
rm -f install-sh
rm -f missing
rm -f depcomp

if [ 0 = 1 ]; then
autoscan
else

touch NEWS
touch README
touch README.md
touch AUTHORS
touch ChangeLog
touch config.h.in
touch install-sh

libtoolize --force --copy --install --automake
aclocal
automake -a -c
autoconf
# run twice to get rid of 'ltmain.sh not found'
autoreconf -f -i -Wall,no-obsolete
autoreconf -f -i -Wall,no-obsolete

if [ 0 = 1 ]; then
  exit 0

  #./configure --prefix="${MY_PREFIX}" --exec-prefix="${MY_PREFIX}" --libdir="${MY_PREFIX}/lib" --enable-debug --disable-shared --enable-static --enable-coverage --enable-valgrind
  ./configure --prefix="${MY_PREFIX}" --exec-prefix="${MY_PREFIX}" --libdir="${MY_PREFIX}/lib" --disable-debug --disable-shared --enable-static --disable-coverage --disable-valgrind

  make clean
  make ChangeLog
  make dist-gzip
  #make -C doc/latex/
else
  DN_CUR=`pwd`

  if [ ! -d cpp-ci-unit-test ]; then
    git clone -q --depth=1 https://github.com/yhfudev/cpp-ci-unit-test.git
  fi
  cd cpp-ci-unit-test
  git checkout master
  git pull
  cd "${DN_CUR}"

  which "$CC" || CC=gcc
  which "$CXX" || if [[ "$CC" =~ .*clang.* ]]; then CXX=clang++; else CXX=g++; fi
  echo "=========== compiling libosporting MY_PREFIX=${MY_PREFIX}"
  CC=$CC CXX=$CXX ./configure --prefix="${MY_PREFIX}" --exec-prefix="${MY_PREFIX}" --libdir="${MY_PREFIX}/lib" --disable-shared --enable-static --disable-debug --enable-coverage --enable-valgrind \
    --with-ciut=`pwd`/cpp-ci-unit-test \
    ${NULL}

  make clean; make -j 8 coverage CC=$CC CXX=$CXX; make check CC=$CC CXX=$CXX; make check-valgrind CC=$CC CXX=$CXX

  #make DESTDIR="`pwd`/lib/" install
  echo "=========== installing libosporting MY_PREFIX=${MY_PREFIX}"
  make install
  echo "=========== DONE installing libosporting MY_PREFIX=${MY_PREFIX}"
fi


fi
