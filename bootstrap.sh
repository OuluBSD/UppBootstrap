#!/usr/bin/env sh
# ==============================================================================
# UppBootstrap: Zero-dependency U++ umk bootstrap toolchain for POSIX systems
# Supports Linux, macOS, FreeBSD, OpenBSD, NetBSD
# ==============================================================================

set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
REPO_ROOT="$SCRIPT_DIR"

SOURCE="$SCRIPT_DIR/src/main.cpp"
OUTPUT="$REPO_ROOT/bin/upp-bootstrap"

mkdir -p "$REPO_ROOT/bin"

CXX_BIN="${CXX:-}"
if [ -z "$CXX_BIN" ]; then
    if command -v clang++ >/dev/null 2>&1; then
        CXX_BIN=clang++
    elif command -v g++ >/dev/null 2>&1; then
        CXX_BIN=g++
    elif command -v c++ >/dev/null 2>&1; then
        CXX_BIN=c++
    else
        echo "Error: No standard C++ compiler found (tried \$CXX, clang++, g++, c++)." >&2
        exit 1
    fi
fi

echo "==> UppBootstrap: Compiling bootstrap runner with $CXX_BIN..."
"$CXX_BIN" -std=c++11 -O2 -Wall -o "$OUTPUT" "$SOURCE"

echo "==> UppBootstrap: Executing bootstrap..."
exec "$OUTPUT" --bootstrap --methods-dir "$SCRIPT_DIR/methods" "$@"
