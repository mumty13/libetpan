#!/bin/sh

# Set up environment
pushd "`dirname "$0"`" > /dev/null
scriptpath="`pwd`"
popd > /dev/null

# Configuration
name="libetpan"
xcode_project="build-mac/libetpan.xcodeproj"
ios_target="libetpan ios"
ios_library="libetpan-ios.a"

# Build settings
simarchs="x86_64"  # Removed arm64 from simulator archs to avoid conflict
devicearchs="arm64"
sdkminversion="13.0"
sdkversion="`xcodebuild -showsdks 2>/dev/null | grep iphoneos | sed 's/.*iphoneos\(.*\)/\1/'`"

# Build directories
builddir="$scriptpath/build"
BUILD_TIMESTAMP=`date +'%Y%m%d%H%M%S'`
tempbuilddir="$builddir/workdir/$BUILD_TIMESTAMP"
tmpdir="$tempbuilddir/tmp"
resultdir="$builddir/result"

# Create necessary directories
mkdir -p "$tempbuilddir"
mkdir -p "$tmpdir/bin"
mkdir -p "$tmpdir/obj"
mkdir -p "$resultdir"

# Log file
logfile="$tempbuilddir/build.log"

echo "Building libetpan for iOS..."
echo "Working in $tempbuilddir"
echo "Log file: $logfile"

# Prepare the build environment
echo "Preparing build environment..." | tee -a "$logfile"
cd "$scriptpath"

# Run autogen if needed
if [ ! -f "$scriptpath/configure" ]; then
    echo "Running autogen.sh..." | tee -a "$logfile"
    ./autogen.sh >> "$logfile" 2>&1
    if [ $? -ne 0 ]; then
        echo "Error: autogen.sh failed" | tee -a "$logfile"
        exit 1
    fi
fi

# Prepare for Xcode build
echo "Preparing for Xcode build..." | tee -a "$logfile"
cd "$scriptpath/build-mac"
./update.sh prepare >> "$logfile" 2>&1
if [ $? -ne 0 ]; then
    echo "Error: update.sh prepare failed" | tee -a "$logfile"
    exit 1
fi

# Prepare iOS dependencies if needed
if [ ! -d "$scriptpath/build-mac/libsasl-ios" ]; then
    echo "Building dependencies for iOS..." | tee -a "$logfile"
    cd "$scriptpath/build-mac"
    ./prepare-ios.sh >> "$logfile" 2>&1
    if [ $? -ne 0 ]; then
        echo "Error: prepare-ios.sh failed" | tee -a "$logfile"
        exit 1
    fi
fi

# Build for iOS devices
echo "Building for iOS devices (arm64)..." | tee -a "$logfile"
cd "$scriptpath"
xcodebuild -project "$xcode_project" -sdk "iphoneos$sdkversion" -scheme "$ios_target" -configuration Release SYMROOT="$tmpdir/bin" OBJROOT="$tmpdir/obj" ARCHS="$devicearchs" IPHONEOS_DEPLOYMENT_TARGET="$sdkminversion" >> "$logfile" 2>&1
if [ $? -ne 0 ]; then
    echo "Error: iOS device build failed" | tee -a "$logfile"
    exit 1
fi

# Build for iOS simulator
echo "Building for iOS simulator (x86_64)..." | tee -a "$logfile"
xcodebuild -project "$xcode_project" -sdk "iphonesimulator$sdkversion" -scheme "$ios_target" -configuration Release SYMROOT="$tmpdir/bin" OBJROOT="$tmpdir/obj" ARCHS="$simarchs" IPHONEOS_DEPLOYMENT_TARGET="$sdkminversion" >> "$logfile" 2>&1
if [ $? -ne 0 ]; then
    echo "Error: iOS simulator build failed" | tee -a "$logfile"
    exit 1
fi

# Create universal iOS library
echo "Creating universal iOS library..." | tee -a "$logfile"
mkdir -p "$resultdir/ios/lib"
mkdir -p "$resultdir/ios/include"
cp -R "$tmpdir/bin/Release-iphoneos/include/libetpan" "$resultdir/ios/include/"

# Create universal library with lipo
lipo -create "$tmpdir/bin/Release-iphoneos/$ios_library" "$tmpdir/bin/Release-iphonesimulator/$ios_library" -output "$resultdir/ios/lib/$ios_library"
if [ $? -ne 0 ]; then
    echo "Warning: Could not create fat binary with lipo, copying device library instead" | tee -a "$logfile"
    cp "$tmpdir/bin/Release-iphoneos/$ios_library" "$resultdir/ios/lib/$ios_library"
    
    # Also provide individual architecture libraries
    mkdir -p "$resultdir/ios/lib/device"
    mkdir -p "$resultdir/ios/lib/simulator"
    cp "$tmpdir/bin/Release-iphoneos/$ios_library" "$resultdir/ios/lib/device/$ios_library"
    cp "$tmpdir/bin/Release-iphonesimulator/$ios_library" "$resultdir/ios/lib/simulator/$ios_library"
fi

# Copy dependencies
if [ -d "$scriptpath/build-mac/libsasl-ios" ]; then
    echo "Copying iOS dependencies..." | tee -a "$logfile"
    mkdir -p "$resultdir/ios/deps"
    cp -R "$scriptpath/build-mac/libsasl-ios" "$resultdir/ios/deps/"
fi

# Create zip archive
echo "Creating zip archive..." | tee -a "$logfile"
cd "$resultdir"
zip -qry "$resultdir/$name-ios.zip" ios

echo "Build completed successfully!" | tee -a "$logfile"
echo "Results available in: $resultdir"
echo "  - iOS: $resultdir/$name-ios.zip"

# Cleanup
echo "Cleaning up..." | tee -a "$logfile"
rm -rf "$tempbuilddir"

exit 0
