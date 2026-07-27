# Network_Lib Notes

## Scope
- This library owns ASIO clients/servers, protobuf message framing, IDs, and game-room server state.
- It must stay independent of SDL/App UI; app-specific network behavior lives in `App_Lib/src/app/cnetwork`.

## Protocol/Framing
- `ProtobufMessage` uses a 2-byte header and max body size `65535`; reserve body size before writing via `getMutableBodyBuffer()`.
- Include generated protobuf as `<protocol/*.pb.h>`; do not depend on generated file paths directly.
- ASIO is standalone: `Network_Lib/CMakeLists.txt` sets target compile definitions `ASIO_STANDALONE` and `ASIO_NO_DEPRECATED`.

## Tests
- Network behavior is mainly covered from `App_Test/src/network`; focused runs use `ctest --test-dir build_debug/App_Test -R 'NetworkTest.testName' --output-on-failure`.
- Tests use mock/debug clients and `asio::io_context`; drain/poll pending work when stopping coroutine-based server code.
