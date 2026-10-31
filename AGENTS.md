# UppBootstrap - Agent Architecture Guide

**For**: AI agents and developers working with `UppBootstrap`

---

## Architecture & Responsibilities

`UppBootstrap` is strictly constrained to **stage 1 bootstrap**:
- Do NOT introduce complex package dependency graph resolvers here (that belongs in `UppBuilder`).
- Do NOT introduce 3-pillar redirects, file overlays, or patch engines here (that belongs in `UppBuilder`).
- Do NOT depend on external runtime interpreters (e.g. Python, Perl, Node). Keep dependencies strictly zero beyond the host system's C++ toolchain and shell.

### Essential Invariants
1. `src/main.cpp` must remain pure C++11 standard library only (`<iostream>`, `<string>`, `<vector>`, `<sys/stat.h>`, etc.).
2. Build methods generated or placed into `~/.config/u++/theide` or `%APPDATA%\u++\theide` must produce identical builder semantics to standard U++ `umk`.
3. Exit codes must follow UNIX standard: 0 on success, non-zero on failure.
