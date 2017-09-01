
dependencies="$(pwd)/dependencies"

rm -rf $dependencies

#zlib
function zlib {
	pushd $(mktemp -d)
	version=$1
	wget https://github.com/madler/zlib/archive/v$version.tar.gz
	tar xzf v$version.tar.gz
	pushd zlib-$version || exit 1
	CC=arm-linux-gnueabihf-gcc ./configure --prefix=$dependencies
	make
	make install || exit 1
	popd
}

# libpng
function libpng {
	pushd $(mktemp -d)
	version=$1
	wget https://github.com/glennrp/libpng/archive/v$version.tar.gz
	tar xzf v$version.tar.gz
	pushd libpng-$version || exit 1
	./autogen.sh
	./configure --host=arm-linux-gnueabihf CC=arm-linux-gnueabihf-gcc \
	    AR=arm-linux-gnueabihf-ar STRIP=arm-linux-gnueabihf-strip RANLIB=arm-linux-gnueabihf-ranlib \
	   CPPFLAGS="-mfpu=neon -I$dependencies/include" LDFLAGS="-L$dependencies/lib"  \
	   --prefix=$dependencies
	make
	make install || exit 1
	popd
}
popd

zlib 1.2.5.3 && libpng 1.6.32