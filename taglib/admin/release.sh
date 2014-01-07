#!/bin/bash

release=1.3.1

echo "********** Copying taglib -> taglib-$release..."

cd taglib
perl admin/cvs-clean.pl
cd ..

rm -rf taglib-$release
cp -a taglib taglib-$release

echo "********** Moving directories from their CVS locations to their release locations..."

cd taglib-$release
mv taglib/examples taglib/tests taglib/bindings .

echo "********** Applying directory patches..."
patch -p0 < admin/make-changes.diff

echo "********** Running automake..."
make -f Makefile.cvs

echo "********** Removing automake cache..."
rm -r autom4te.cache

echo "********** Removing CVS and .cvsignore files..."
find . -name CVS -type d | xargs rm -r
find . -name .cvsignore -type f | xargs rm

echo "********** Creating tarball..."

cd ..
tar cfz taglib-$release.tar.gz taglib-$release

echo "********** Checking build..."

cd taglib-$release
./configure && make -j 2 && make check && make examples &&
echo "********** Running toolkit test suite..." &&
./tests/toolkit-test &&
cd ..
echo "********** Done"
