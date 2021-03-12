make clean

export NDKROOT=$PWD/android-ndk-r10c

# For ARM 
#Check cc - ex) /home/shin/Downloads/android-ndk-r10c/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc
#export AR=$NDKROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-ar
#export LD=$NDKROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-ld
#export CC=$NDKROOT/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc

# For x86 
#export AR=$NDKROOT/toolchains/x86-4.4.3/prebuilt/linux-x86/bin/i686-android-linux-ar
#export LD=$NDKROOT/toolchains/x86-4.4.3/prebuilt/linux-x86/bin/i686-android-linux-ld
#export CC=$NDKROOT/toolchains/x86-4.4.3/prebuilt/linux-x86/bin/i686-android-linux-gcc

export AR=$NDKROOT/toolchains/x86-4.9/prebuilt/linux-x86_64/bin/i686-linux-android-ar
export LD=$NDKROOT/toolchains/x86-4.9/prebuilt/linux-x86_64/bin/i686-linux-android-ld
export CC=$NDKROOT/toolchains/x86-4.9/prebuilt/linux-x86_64/bin/i686-linux-android-gcc

./autogen.sh

#CPPFLAGS="--sysroot=$NDKROOT/platforms/android-14/arch-arm" \
#   CFLAGS="--sysroot=$NDKROOT/platforms/android-14/arch-arm" \
#   ./configure --prefix=/data/local/tmp/Inst \
#   --host=armv7-unknown-linux --target=armv7-unknown-linux \
#   --with-tmpdir=/sdcard

#CPPFLAGS="--sysroot=$NDKROOT/platforms/android-9/arch-x86" \
#   CFLAGS="--sysroot=$NDKROOT/platforms/android-9/arch-x86 -fno-pic" \
#   ./configure --prefix=/data/local/Inst \
#   --host=i686-android-linux --target=i686-android-linux \
#   --with-tmpdir=/sdcard

CPPFLAGS="--sysroot=$NDKROOT/platforms/android-14/arch-x86" \
   CFLAGS="--sysroot=$NDKROOT/platforms/android-14/arch-x86 -fno-pic" \
   ./configure --prefix=/data/local/tmp/Inst \
   --host=i686-unknown-linux --target=i686-unknown-linux \
   --with-tmpdir=/sdcard

s1='DEFAULT_INCLUDES = -I. -I$(top_builddir)'
s3='DEFAULT_INCLUDES = -I. -I$(top_builddir) -I..\/..\/..\/include -I..\/VEX\/priv -I$(top_builddir)\/coregrind'

sed -i "s/$s1/$s3/g" "none/Makefile"

make -j4

make -j4 install DESTDIR=`pwd`/Inst
