#!/bin/bash
set -e

# Detect NDK Location
NDK_PATH="/Users/cashify/Library/Android/sdk/ndk/25.2.9519653/ndk-build"

if [ ! -f "$NDK_PATH" ]; then
    NDK_PATH=$(which ndk-build 2>/dev/null || echo "")
fi

if [ -z "$NDK_PATH" ]; then
    echo "Error: ndk-build not found!"
    exit 1
fi

echo "Building native binary using NDK..."
"$NDK_PATH" -C app/src/main

echo "Build successful! Output placed in app/src/main/libs/arm64-v8a/shadow.sh"
