# cambrian-android

### Getting set up

This repo can be automatically downloaded by following the common-core setup and running `cb download android`

Download the Android NDK: https://developer.android.com/ndk/downloads/index.html

Add the following to your `.bash_profile`. Edit paths as necessary.

```
export ANDROID_NDK=/mnt/c/cambrian/NDK/linux/android-ndk-r15

# Optional
#export CBAN=$CB/cambrian-android/src/cambrian-android

# Old build required paths
#export ANDROID_NDK_ROOT=/mnt/c/cambrian/NDK/linux/android-ndk-r15
#export ANDROID_NDK_CLANG=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64
#export ANDROID_NDK_ARM=$ANDROID_NDK_ROOT/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64
#export ANDROID_NDK_X86=$ANDROID_NDK_ROOT/toolchains/x86-4.9/prebuilt/linux-x86_64
```


Once the paths are set, run `cb build core android` to build the Android binary from core.

