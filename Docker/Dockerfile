FROM mcr.microsoft.com/devcontainers/cpp:debian

WORKDIR /app/src

RUN git clone https://github.com/mwthinker/MWetris.git MWetris

WORKDIR /app/src/MWetris

RUN cmake --preset="unix-game-server-only" -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=1 && cmake --build build
RUN cmake --build build --config Release

RUN mkdir -p /app/bin && mv build/GameServer/GameServer /app/bin/.

WORKDIR /app/bin/

ENTRYPOINT ["./GameServer"]
