#!/bin/sh

# Function to display usage instructions
usage() {
    echo "Usage: $0 [debug|release]"
    exit 1
}

# Check if the first argument is supplied
if [ -z "$1" ]; then
    usage
fi

# Determine the build mode based on the argument
case "$1" in
    debug)
        CONFIG_PRESET="ux-debug"
        BUILD_PRESET="ux-debug-build"
        ;;
    release)
        CONFIG_PRESET="ux-release"
        BUILD_PRESET="ux-release-build"
        ;;
    *)
        usage
        ;;
esac

# Create the build directory if it doesn't exist
BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

# Configure and build the project using the appropriate presets
cmake --preset=$CONFIG_PRESET
cmake --build --preset=$BUILD_PRESET

echo "Build complete in $1 mode!"
