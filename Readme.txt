defconfig file: sdm845_defconfig

Download:
========
Download the corresponding official android toolchain:

linux-x86:
git clone https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9

darwin-x86:
git clone https://android.googlesource.com/platform/prebuilts/gcc/darwin-x86/aarch64/aarch64-linux-android-4.9

Build the kernel:
=================
set the following environment variables:

$ mkdir -p out
$ make ARCH=arm64 CROSS_COMPILE=<path-to-toolchain>/bin/aarch64-linux-android- O=out sdm845_defconfig
make ARCH=arm64 CROSS_COMPILE=/data/Android/build/omni/android-8.1/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android- O=out sdm845_defconfig

$ make ARCH=arm64 CROSS_COMPILE=<path-to-toolchain>/bin/aarch64-linux-android- O=out -j16 DTC_EXT=/media/opensource/T4/workspace_T4/IMAGINE_DUGL_O80_SENSE10GP/836/patch/dtc CONFIG_BUILD_ARM64_DT_OVERLAY=y   
make ARCH=arm64 CROSS_COMPILE=/data/Android/build/omni/android-8.1/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin/aarch64-linux-android- O=out -j4 DTC_EXT=../dtc CONFIG_BUILD_ARM64_DT_OVERLAY=y

Output binary files:
====================
After the build process finishes, the kernel image will be located at out/arch/arm64/boot/Image.lz4-dtb


For additional information:
===========================
http://htcdev.com

