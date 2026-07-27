# Protocol Notes

## Generated Code
- Edit `*.proto` sources here; never edit generated `*.pb.*` under the build tree.
- CMake generates headers into `${build}/Protocol/src/protocol`; include them as `#include <protocol/name.pb.h>`.

## Packages
- Shared/high-score messages use proto package `tp`.
- Client-to-server wrapper uses package `tp_c2s`; server-to-client wrapper uses package `tp_s2c`.
- `Network_Lib` and `App_Lib` depend on wrapper field names; update tests when adding/changing message fields.

## Proto Conventions
- `tp.BlockType` enum numeric values intentionally match ASCII block characters; do not renumber casually.
- Keep field numbers stable for existing protocol fields; add new fields with new numbers.
