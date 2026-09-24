#!/usr/bin/env sh
set -eu
: "${QT_ANDROID_ROOT:?Qt 6.11.1 Android ABI kit required}"
: "${QT_HOST_PATH:?Qt 6.11.1 host kit required}"
: "${ANDROID_SDK_ROOT:?Android SDK required}"
abi="${1:-arm64-v8a}"
case "$abi" in arm64-v8a|x86_64) ;; *) echo 'Supported build configurations: arm64-v8a or x86_64' >&2; exit 2;; esac
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
ndk="$ANDROID_SDK_ROOT/ndk/27.2.12479018"
test -f "$ndk/source.properties"
"$QT_ANDROID_ROOT/bin/qt-cmake" -S "$root" -B "$root/build-android-$abi" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DANDROID_ABI="$abi" \
  -DANDROID_NDK="$ndk" -DQT_HOST_PATH="$QT_HOST_PATH" \
  -DQT_ANDROID_MIN_SDK_VERSION=28 -DQT_ANDROID_TARGET_SDK_VERSION=36
cmake --build "$root/build-android-$abi" --target apk
cmake --build "$root/build-android-$abi" --target aab
echo 'Compilation does not establish SAF, lifecycle, 16 KiB or workflow parity. Run device tests before making a support claim.'
