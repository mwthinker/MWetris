---
name: build-mwetris
description: CMake, vcpkg, Windows, Linux, Git Bash, build folder, and test instructions for compiling MWetris. Use when asked to configure, compile, test, or troubleshoot this project.
---

# Build MWetris

Use this skill only for this repository. Run commands from the repository root.

## Prerequisites

- CMake 4.2 or later, Git, and a C++ toolchain are installed.
- vcpkg is bootstrapped and `VCPKG_ROOT` points to its root directory. Project presets require it.
- Initial configuration fetches pinned dependencies, so network access is required.

## Windows

Install Visual Studio 2026 with the **Desktop development with C++** workload and x64 MSVC tools. The `windows` preset uses the `Visual Studio 18 2026` generator, x64, and the `App/vcpkg.json` manifest.

```bash
cmake --preset windows -B build
cmake --build build --config Debug --parallel
```

For a release build:

```bash
cmake --build build --config Release --parallel
```

Visual Studio is multi-config. Do not set `CMAKE_BUILD_TYPE`; select `Debug`, `Release`, `RelWithDebInfo`, or `MinSizeRel` with `--config`.

Git Bash works as the shell when using the Windows preset and `cmake.exe`; it still builds with MSVC. Set `VCPKG_ROOT` using a Windows-compatible path, such as `export VCPKG_ROOT='C:/src/vcpkg'`. Launch Git Bash from a Visual Studio Developer Command Prompt if MSVC cannot be found.

Do not use MinGW, `MinGW Makefiles`, or Ninja plus MinGW with the shipped Windows preset. Its `x64-windows` vcpkg artifacts target MSVC. Native MinGW support requires a dedicated vcpkg triplet and CMake preset.

## Linux

Install the required system packages first:

```bash
sudo apt-get install -y libltdl-dev libx11-dev libxft-dev libxext-dev
```

Configure and build the full application with the Unix Makefiles preset:

```bash
cmake --preset unix -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

Use `Release` for an optimized build:

```bash
cmake --preset unix -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel
```

The Unix generator is single-config; set `CMAKE_BUILD_TYPE` while configuring and do not use `--config`.

## Tests

The normal presets include `App_Test`. Windows:

```bash
ctest --test-dir build/App_Test --output-on-failure -C Debug
```

Linux:

```bash
ctest --test-dir build/App_Test --output-on-failure
```

For Tetris engine tests on Windows, configure a separate directory:

```bash
cmake --preset windows -B build-tests -DTetrisEngine_Test=1
cmake --build build-tests --config Debug --parallel
ctest --test-dir build-tests/TetrisEngine/TetrisEngine_Test --output-on-failure -C Debug
ctest --test-dir build-tests/App_Test --output-on-failure -C Debug
```

On Linux, configure Debug explicitly and omit `-C Debug`:

```bash
cmake --preset unix -B build-tests -DCMAKE_BUILD_TYPE=Debug -DTetrisEngine_Test=1
cmake --build build-tests --parallel
ctest --test-dir build-tests/TetrisEngine/TetrisEngine_Test --output-on-failure
ctest --test-dir build-tests/App_Test --output-on-failure
```

## Server Only

Windows:

```bash
cmake --preset windows-game-server-only -B build-server
cmake --build build-server --config Release --parallel
```

Linux:

```bash
cmake --preset unix-game-server-only -B build-server -DCMAKE_BUILD_TYPE=Release
cmake --build build-server --parallel
```

## Troubleshooting

- `VCPKG_ROOT` is unset: set it to the vcpkg checkout, not the executable or `scripts` directory.
- Generator differs: use a fresh build directory; never reuse one across Visual Studio and Unix/MinGW generators.
- `Visual Studio 18 2026` unavailable: install the C++ workload and verify with `cmake --list-presets`.
- Dependency fetch fails: verify Git and network access, then rerun configure.
