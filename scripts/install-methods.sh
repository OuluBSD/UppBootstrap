#!/usr/bin/env sh
# ==============================================================================
# install-methods.sh: Install U++ build methods (.bm) to system config directories
# ==============================================================================

set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
METHODS_DIR="$SCRIPT_DIR/methods"

if [ ! -d "$METHODS_DIR" ]; then
    echo "Error: methods directory not found at $METHODS_DIR" >&2
    exit 1
fi

HOME_DIR="${HOME:-/}"
CONFIG_DIRS="
$HOME_DIR/.config/u++/theide
$HOME_DIR/.config/u++/umk
$HOME_DIR/.upp/theide
$HOME_DIR/.upp/umk
"

echo "==> Installing U++ build methods..."
INSTALLED=0

for target in $CONFIG_DIRS; do
    mkdir -p "$target"
    for bm in "$METHODS_DIR"/*.bm; do
        if [ -f "$bm" ]; then
            fn=$(basename "$bm")
            cp -f "$bm" "$target/$fn"
            INSTALLED=$((INSTALLED + 1))
        fi
    done
done

echo "==> Successfully installed build methods to:"
echo "    - $HOME_DIR/.config/u++/theide"
echo "    - $HOME_DIR/.upp/theide"
echo "==> umk is now configured with CLANG, GCC, and CLANG_macOS build methods."
