# App Data Notes

## Config Files
- `App/CMakeLists.txt` copies `tetris.json` into the app build directory as the default runtime config.
- Keep `tetris.json` valid against `tetris.schema.json`; schema has `additionalProperties: false` at the root.
- AI choices `ai1`..`ai4` name active AIs; custom AI formulas live under `ais[].valueFunction`.

## Runtime Selection
- `USE_APPLICATION_JSON` is a runtime file, not a CMake flag. Its value `1` means load/save `tetris.json` next to the executable.
- `package/USE_APPLICATION_JSON` value `0` is installed for packages so user config saves under SDL pref path instead.

## Asset Paths
- Paths in JSON are runtime-relative after CMake copies `App/data`, fetched `MWetrisData`, and `gamecontrollerdb.txt` into the app build dir.
