#!/bin/sh

build_git_ios()
{
    scriptpath="$(cd "$(dirname "$0")"; pwd)"
    tmpdir="$scriptpath/tmp-build"
    rm -rf "$tmpdir"
    mkdir -p "$tmpdir"

    echo "Building in $scriptpath"

    # === BITCODE FLAGS ===
    BITCODE_FLAGS="-fembed-bitcode"
    XCODE_BITCODE_FLAGS=""
    if [ -n "$NOBITCODE" ]; then
    BITCODE_FLAGS=""
    XCODE_BITCODE_FLAGS="ENABLE_BITCODE=NO"
    fi

    XCTOOL_OTHERFLAGS='$(inherited)'" $BITCODE_FLAGS"

    # === BUILD FOR DEVICE ===
    sdkversion=$(xcrun --sdk iphoneos --show-sdk-version)
    sdk="iphoneos$sdkversion"
    echo "Building for $sdk"
    xcodebuild -project "$xcode_project" \
    -sdk $sdk \
    -scheme "$xcode_target" \
    -configuration Release \
    SYMROOT="$tmpdir/bin" \
    OBJROOT="$tmpdir/obj" \
    ARCHS="$devicearchs" \
    IPHONEOS_DEPLOYMENT_TARGET="$sdkminversion" \
    OTHER_CFLAGS="$XCTOOL_OTHERFLAGS" \
    $XCODE_BITCODE_FLAGS
    if [ $? -ne 0 ]; then
    echo "Device build failed"
    exit 1
    fi

    # === BUILD FOR SIMULATOR ===
    sdkversion=$(xcrun --sdk iphonesimulator --show-sdk-version)
    sdk="iphonesimulator$sdkversion"
    echo "Building for $sdk"
    xcodebuild -project "$xcode_project" \
    -sdk $sdk \
    -scheme "$xcode_target" \
    -configuration Release \
    SYMROOT="$tmpdir/bin" \
    OBJROOT="$tmpdir/obj" \
    ARCHS="$simarchs" \
    IPHONEOS_DEPLOYMENT_TARGET="$sdkminversion" \
    OTHER_CFLAGS='$(inherited)'
    if [ $? -ne 0 ]; then
    echo "Simulator build failed"
    exit 1
    fi

    # === LIPO COMBINE ===
    mkdir -p "$scriptpath/build"
    lipo -create \
    "$tmpdir/bin/Release-iphoneos/$name.a" \
    "$tmpdir/bin/Release-iphonesimulator/$name.a" \
    -output "$scriptpath/build/$name-universal.a"

    echo "✅ Universal static library created at: build/$name-universal.a"

    # === CLEANUP ===
    rm -rf "$tmpdir"
}