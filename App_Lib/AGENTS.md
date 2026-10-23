# App_Lib Notes

## Boundaries
- `src/app/game` is local gameplay/controller state; core board rules stay in `TetrisEngine/srcLib`.
- `src/app/cnetwork` translates app events to/from `Network_Lib` and protobuf wrappers.
- `src/app/ui` and `src/app/graphic` own ImGui/SDL presentation; launcher/window setup is in sibling `App`.
- Tests for this library live in sibling `App_Test`, not inside `App_Lib`.

## Configuration
- Runtime config is centralized in `app::Configuration`; its constructor reads `USE_APPLICATION_JSON` before choosing the JSON path.
- `USE_APPLICATION_JSON` is a runtime file, not a CMake flag. A value of `1` uses `tetris.json` next to the executable; packaged installs use `App/data/package/USE_APPLICATION_JSON=0` to save under SDL pref path.
- `Configuration::getAiVector()` appends the built-in `Default` AI to JSON-defined AIs; do not duplicate `Default` in JSON unless intended as a separate entry.

## Fonts
- `Configuration::initImGuiFonts()` loads Inter Regular as the default ImGui font, then merges Font Awesome into that font only.
- Fonts returned by `getImGuiRegularSmallFont()`, `getImGuiMediumFont()`, `getImGuiMediumValueFont()`, `getImGuiSemiBoldFont()`, and `getImGuiScoreFont()` do not contain the merged icon glyphs.
- Rendering an `ICON_FA_*` string while one of those non-default fonts is pushed displays a missing-glyph question mark. Render the icon separately with the default font or use font-independent text markers.
- Runtime font files are executable-relative under `fonts/inter`; asset copying and packaging are configured in `App/CMakeLists.txt` and `App/CPack.cmake`.

## Event Flow
- `TetrisController` bridges devices, game rules, network events, and graphic rendering; add cross-cutting game flow there before scattering state across scenes.
- Signals use `mw::signals::ScopedConnections`; keep connection lifetime tied to owning objects.
