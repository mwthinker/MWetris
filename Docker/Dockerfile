FROM mcr.microsoft.com/devcontainers/cpp:debian

WORKDIR /app/src
RUN git clone https://github.com/mwthinker/MWetris.git MWetris

WORKDIR /app/src/MWetris
RUN cd ./Docker; ls -ltr;
RUN chmod +x Docker/build-server; Docker/build-server
RUN mkdir -p /app/bin && mv build/GameServer/GameServer /app/bin/.

WORKDIR /app/bin/

ENTRYPOINT ["./GameServer"]
