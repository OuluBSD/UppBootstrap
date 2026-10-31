# UppBootstrap

**Zero-dependency U++ `umk` bootstrap toolchain for all system toolchains and shells.**

Part of the **OuluBSD U++ Modularization Architecture**:
1. **`UppBootstrap`** (this repo) — Zero-dependency toolchain bootstrap (`umk` compiler & build methods).
2. **`UppBuilder`** — Advanced builder with the 3-pillar bridge (redirects, overlays, patches).
3. **`BazaarCore`** — Modularized subpackages (`Core/*`, `Ctrl/{Core,Lib}`, `Draw/*`) with surgical upstream patches and multi-root UppHub registry.

---

## 🎯 Purpose & Scope

The sole mission of `UppBootstrap` is to bring up the U++ build system on an empty system without requiring:
- Any pre-existing U++ installation or TheIDE binary
- Any external language runtime (zero Python, Perl, Node, Ruby)
- Any third-party build tools (works purely with standard system C++ compilers and shells)

It performs three distinct stages:
1. **Toolchain Detection & Build Method Provisioning**: Detects host C++ compiler (`clang++`, `g++`, `cl.exe`) and generates canonical `.bm` method files (`CLANG.bm`, `GCC.bm`, `MSVS22x64.bm`, `CLANG_macOS.bm`) in `~/.config/u++/theide/`, `~/.upp/theide/` or `%APPDATA%\u++\theide\`.
2. **Resource Transpilation**: Converts U++ `.brc` binary resource manifests into `.brcc` C++ compilation units.
3. **`umk` Build & Verification**: Executes the platform Makefile (`Makefile.linux64`, `Makefile.macos15`) or MSVC compilation unit to produce `bin/umk` (`bin/umk.exe`) and verifies functional execution via `--help`.

---

## 🚀 Quick Start

### POSIX (Linux, FreeBSD, OpenBSD, NetBSD, macOS)

```bash
git clone https://github.com/OuluBSD/UppBootstrap.git
cd UppBootstrap

# Point to your U++ source tree (defaults to sibling ai-upp or ../uppsrc)
./bootstrap.sh --upp-root /path/to/ai-upp
```

### Windows (PowerShell)

```powershell
git clone https://github.com/OuluBSD/UppBootstrap.git
cd UppBootstrap

# Detects Visual Studio MSVC, Clang, or MinGW automatically
.\bootstrap.ps1 -UppRoot C:\Dev\ai-upp
```

### Windows (CMD Batch)

```cmd
git clone https://github.com/OuluBSD/UppBootstrap.git
cd UppBootstrap

bootstrap.cmd --upp-root C:\Dev\ai-upp
```

---

## 📂 Repository Layout

```
UppBootstrap/
├── bootstrap.sh            # POSIX shell entrypoint (sh / bash)
├── bootstrap.ps1           # Windows PowerShell entrypoint (MSVC auto-discovery)
├── bootstrap.cmd           # Windows CMD batch entrypoint
├── Makefile                # Minimal Makefile wrapper
├── src/
│   └── main.cpp            # Zero-dependency C++11 toolchain & umk builder
├── methods/                # Canonical U++ build method templates
│   ├── CLANG.bm
│   ├── GCC.bm
│   ├── MSVS22x64.bm
│   └── CLANG_macOS.bm
├── .github/
│   └── workflows/
│       └── ci.yml          # Multi-OS CI matrix (Linux, macOS, Windows)
├── AGENTS.md               # Technical notes for AI agents
└── README.md
```

---

## 🛠️ CLI Options

```
Usage: upp-bootstrap [options]

Options:
  --bootstrap, -bs      Bootstrap umk and configure default methods (default action)
  --upp-root PATH       Root directory of U++ source tree (default: auto-detected)
  --methods-dir PATH    Directory containing template .bm methods
  --install-methods     Only install .bm build methods to host config
  --verify              Verify umk executable execution
  --verbose, -v         Enable verbose diagnostic logs
  --help, -h            Show help documentation
```

---

## 📜 License

BSD-2-Clause / Ultimate++ License.
