#!/bin/sh

pushd "`dirname "$0"`" > /dev/null
scriptpath="`pwd`"
popd > /dev/null

. "$scriptpath/build-libetpan.sh"

url="https://github.com/mumty13/libetpan.git"
rev=6c09c1b7367f3f725114acdb19b46d1c1ad84dea
name="libetpan-ios"
xcode_target="libetpan ios"
xcode_project="libetpan.xcodeproj"
library="libetpan-ios.a"
embedded_deps="libsasl-ios"

build_git_ios
