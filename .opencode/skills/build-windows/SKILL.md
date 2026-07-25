---
name: build-windows
description: Windows, Visual Studio 2026, CMake, vcpkg, Git Bash, and build folder instructions for compiling MWetris. Use when asked to configure, compile, test, or troubleshoot this project on Windows.
---

# Build MWetris On Windows

Use this skill only for this repository on Windows.

## Prerequisites

- CMake 4.2 or later is installed and on `PATH`.
- Visual Studio 2026 is installed with the **Desktop development with C++** workload and x64 MSVC tools.
- vcpkg is bootstrapped and `VCPKG_ROOT` points to its root directory. The project's presets require it.
- Git is available. Initial configuration fetches pinned dependencies, so network access is required.

Run from the repository root. The existing `windows` preset uses the `Visual Studio 18 2026` generator, x64 architecture, and `App/vcpkg.json` manifest.

## Configure And Build

Use the repository's `build` directory:

```bash
cmake --preset windows -B build
cmake --build build --config Debug --parallel
```

For an optimized build:

```bash
cmake --build build --config Release --parallel
```

Visual Studio is a multi-config generator. Do not pass `-DCMAKE_BUILD_TYPE`; choose `Debug`, `Release`, `RelWithDebInfo`, or `MinSizeRel` with `--config` when building.

To rebuild configuration after changing preset, generator, architecture, or vcpkg settings, use a new build directory. Do not reuse `build` for a different generator.

## Tests

The normal Windows preset includes `App_Test`. Run tests after a Debug build:

```bash
ctest --test-dir build/App_Test --output-on-failure -C Debug
```

To include Tetris engine tests, configure separately:

```bash
cmake --preset windows -B build-tests -DTetrisEngine_Test=1
cmake --build build-tests --config Debug --parallel
ctest --test-dir build-tests/TetrisEngine/TetrisEngine_Test --output-on-failure -C Debug
ctest --test-dir build-tests/App_Test --output-on-failure -C Debug
```

## Server Only

Use the server manifest and omit the GUI:

```bash
cmake --preset windows-game-server-only -B build-server
cmake --build build-server --config Release --parallel
```

## Git Bash / MinGW

Git Bash is supported as the shell when it runs Windows `cmake.exe` and the project's `windows` preset:

```bash
export VCPKG_ROOT='C:/src/vcpkg'
cmake --preset windows -B build
cmake --build build --config Debug --parallel
```

This still compiles with MSVC and builds Visual Studio projects. Launching Git Bash from a Visual Studio Developer Command Prompt is the most reliable option if MSVC environment variables are not otherwise available.

Do not configure this project with Git Bash's MinGW compiler, `MinGW Makefiles`, or Ninja plus MinGW unless a dedicated MinGW triplet and preset are added. The shipped preset and current `build/vcpkg_installed/x64-windows` dependencies target MSVC, so mixing MinGW causes incompatible compiler, ABI, and vcpkg artifacts.

## Troubleshooting

- `VCPKG_ROOT` is unset: set it to the vcpkg checkout, not its executable or `scripts` directory.
- CMake says the generator differs: use a fresh build directory or remove only the build directory after confirming it contains no needed artifacts.
- `Visual Studio 18 2026` is unavailable: install the C++ workload, then verify with `cmake --help` or `cmake --list-presets`.
- Dependency fetch failures: verify Git and network access, then rerun configure.
