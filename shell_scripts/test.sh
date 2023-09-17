#!/bin/bash
# Run tests.

# You can specify build type as an argument like "bash test.sh Debug"
if [ "$1" = "Debug" ]; then
    build_type="Debug"
    options="-Dbuildtype=debug -Dlibui:buildtype=debug"
else
    build_type="Release"
    options="-Dbuildtype=custom -Dlibui:buildtype=custom -Db_ndebug=true -Dcpp_rtti=false -Db_lto=true"
    options="${options} -Dcpp_eh=none -Dlibui:cpp_eh=none -Ddebug=false -Doptimization=s"
fi
echo "Build type: ${build_type}"

if [[ "$OSTYPE" == "darwin"* ]]; then
    # You can build universal binaries with the secound argument like "bash test.sh Release Universal"
    if [ "$2" = "Universal" ]; then
        echo "Universal build: On"
    else
        options="${options} -Dmacosx_build_universal=false"
        echo "Universal build: Off"
    fi
fi

common_opt="-Ddefault_library=static -Dlibui:default_library=static
 -Dbuild_exe=false -Dbuild_test=true
 -Dlibui:tests=false -Dlibui:examples=false -Db_coverage=true"

# Build and test
pushd $(dirname "$0")/..
    meson setup build/${build_type}-Test ${common_opt} ${options} || exit 1
    cd build/${build_type}-Test
    meson compile -v || exit 1
    meson test -v || exit 1
popd
