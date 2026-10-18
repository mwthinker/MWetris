# MWetris Agent Notes

## Build Setup
- Requires CMake `>=4.2` and `VCPKG_ROOT`; presets use `$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake`.
- Linux full-app builds need system libs from CI: `sudo apt-get install -y libltdl-dev libx11-dev libxft-dev libxext-dev`.
- `CMakePresets.json` sets full-app `VCPKG_MANIFEST_DIR` to `App`; `*-game-server-only` presets set `GAME_SERVER_ONLY=ON` and use `GameServer`.
- `ExternalFetchContent.cmake` fetches pinned repos during configure: `SDL_GameControllerDB`, `IconFontCppHeaders`, `CMakeHelper`, `MWetrisData`.

## Commands
- CI debug configure: `cmake --preset=unix -B build_debug -DTetrisEngine_Test=1 -DUSE_WIX_GENERATOR=1 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=1 -DCODE_COVERAGE=1`
- CI debug build: `cmake --build build_debug --config Debug`
- CI debug tests: `ctest --rerun-failed --output-on-failure --test-dir build_debug/TetrisEngine/TetrisEngine_Test/` then `ctest --rerun-failed --output-on-failure --test-dir build_debug/App_Test/`
- CI release configure/build mirrors debug with `build_release`, `Release`, and no `CODE_COVERAGE`.
- Server-only release: `cmake --preset=unix-game-server-only -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=1 && cmake --build build --config Release`.
- Focus a GTest case with `ctest --test-dir build_debug/App_Test -R 'SuiteName.testName' --output-on-failure` or the equivalent `TetrisEngine/TetrisEngine_Test` test dir.

## Project Shape
- `TetrisEngine/srcLib` is the core Tetris rules/AI library; `TetrisEngine/src` is the CLI simulator.
- `Protocol/CMakeLists.txt` generates C++ from `Protocol/*.proto` under the build tree; include generated headers as `protocol/*.pb.h`, do not edit generated files.
- `Network_Lib` depends on `Protocol` and `TetrisEngine_Lib`; `GameServer` is a TCP server on default port `11175`.
- `App_Lib` contains game/UI/network client logic; `App` is the SDL GUI launcher and outputs executable name `MWetris`.
- `App_Test` is included in full builds; `TetrisEngine_Test` and `TetrisEngine_Benchmark` require `-DTetrisEngine_Test=1` / `-DTetrisEngine_Benchmark=1`.

## Runtime/Data Gotchas
- App runtime data is copied at build time from `App/data`, fetched `MWetrisData`, and fetched `SDL_GameControllerDB` into the app build dir.
- Fonts come from the pinned `MWetrisData` repository, not `App/data`. Inter is stored as `fonts/inter/{Inter_18pt-Regular.ttf,Inter_18pt-Medium.ttf,Inter_18pt-SemiBold.ttf,OFL.txt}`; `App/CMakeLists.txt` copies that tree to `<build>/App/fonts/inter`, and `App/CPack.cmake` installs it with the same layout. Runtime font paths are executable-relative (`fonts/inter/...`). After changing the `MWetrisData` hash in `ExternalFetchContent.cmake`, rerun CMake configure to fetch and copy the new assets.
- `App/data/tetris.json` is schema-backed by `App/data/tetris.schema.json`; AI value functions live in this JSON.
- Docker server flow is `Docker/build-server`; on arm64 it exports `VCPKG_FORCE_SYSTEM_BINARIES=1`, builds `unix-game-server-only`, and compose maps port `11175`.

## Style
- Follow `.github/copilot-instructions.md`; notable repo conventions: C++23 without modules, tabs for indentation, classic header guards, trailing `_` members, compact nested namespaces.
