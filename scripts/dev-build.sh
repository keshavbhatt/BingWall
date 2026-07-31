#!/usr/bin/env bash
# Builds bing-wall on the host against the KDE Qt 6.11 snap SDK
# (kde-qt6-core24-sdk) — the same Qt the shipped snap consumes at runtime
# from the kf6-core24 content snap (dev/prod parity).
set -euo pipefail

DIR="$(cd "$(dirname "$0")/.." && pwd)"
SDK=/snap/kde-qt6-core24-sdk/current

if [ ! -d "$SDK/usr/lib/x86_64-linux-gnu/cmake/Qt6" ]; then
    echo "Qt SDK snap missing. Install with:" >&2
    echo "  sudo snap install kde-qt6-core24-sdk" >&2
    exit 1
fi

# The SDK snap's build tools (moc, rcc, uic) need the SDK's own Qt
# libraries, not the host's.
export LD_LIBRARY_PATH="$SDK/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

cmake -S "$DIR" -B "$DIR/build/dev" \
    -DCMAKE_BUILD_TYPE="${BINGWALL_BUILD_TYPE:-RelWithDebInfo}" \
    -DCMAKE_PREFIX_PATH="$SDK/usr" \
    -DQt6_DIR="$SDK/usr/lib/x86_64-linux-gnu/cmake/Qt6" \
    -DCMAKE_EXE_LINKER_FLAGS="-Wl,-rpath-link,$SDK/usr/lib/x86_64-linux-gnu -Wl,--allow-shlib-undefined" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$DIR/build/dev" -j"$(nproc)"
