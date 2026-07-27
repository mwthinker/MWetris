# TetrisEngine Notes

## Scope
- `srcLib/tetris` is the reusable rules/board/AI library consumed by app, network, tests, and CLI.
- `src` is only the CLI simulator; keep app/UI or network behavior out of `srcLib`.

## Build/Test
- `TetrisEngine_Test` is opt-in: configure with `-DTetrisEngine_Test=1` before expecting `build_debug/TetrisEngine/TetrisEngine_Test/` to exist.
- Focus tests with `ctest --test-dir build_debug/TetrisEngine/TetrisEngine_Test -R 'TetrisTest.testName' --output-on-failure`.
- Benchmarks are also opt-in with `-DTetrisEngine_Benchmark=1`.

## CLI Gotchas
- CLI flags are custom-parsed in `src/flags.cpp`, not argparse.
- Random-file input for `-f/--file-data` expects block numbers `0..6` (`I,J,L,O,S,T,Z`) and loops back to file start at EOF.
- AI value functions are parsed by `Calculator::Calculator`; validate formula changes with `TetrisEngine --help` or a short simulator run.
