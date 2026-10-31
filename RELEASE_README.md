# UppBootstrap: umk Standalone Distribution

This release archive contains the standalone **`umk`** (U++MaKe) binary, default build methods (`.bm`), and an automatic installation script.

---

## 1. Quick Setup

To allow `umk` to locate host compilers, install the included build methods to your user configuration directory:

### On Linux / macOS / BSD:
```bash
chmod +x install-methods.sh umk
./install-methods.sh
```

### On Windows:
```cmd
install-methods.cmd
```

The script installs:
- `CLANG.bm`: Clang / LLVM toolchain configuration
- `GCC.bm`: GCC / G++ toolchain configuration
- `CLANG_macOS.bm`: macOS Apple Clang configuration
- `MSVS22x64.bm`: Visual Studio 2022 (MSVC x64) configuration

Configuration destinations:
- **POSIX**: `~/.config/u++/theide/` and `~/.upp/theide/`
- **Windows**: `%APPDATA%\u++\theide\` and `%USERPROFILE%\upp\`

---

## 2. Basic Usage

Run `umk` directly or copy it into your `PATH` (e.g. `/usr/local/bin` or `C:\Windows\System32`).

### Command Syntax:
```bash
umk <assembly> <main_package> [build_method] [-options] [+flags] [output_path]
```

### Common Examples:
```bash
# Build the Core package using CLANG in debug mode with blitz:
./umk /path/to/uppsrc Core CLANG -b /tmp/core_test

# Rebuild all in Release mode using GCC:
./umk /path/to/uppsrc MyApp GCC -abr ~/bin/MyApp

# Check missing external dependencies:
./umk /path/to/uppsrc MyApp CLANG -E

# Update UppHub packages without compiling:
./umk /path/to/uppsrc MyApp CLANG --hub-only -U
```

---

## 3. Customizing Build Methods

Build methods are plain-text `.bm` files located in your config directory. You can adjust compiler flags, include paths, optimization settings, or debugger options by editing them directly.
