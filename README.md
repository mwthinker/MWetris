tetris::MWetris [![CI build](https://github.com/mwthinker/mwetris/actions/workflows/ci.yml/badge.svg)](https://github.com/mwthinker/mwetris/actions/workflows/ci.yml) [![codecov](https://codecov.io/gh/mwthinker/MWetris/graph/badge.svg?token=ULQ0QLVVY4)](https://codecov.io/gh/mwthinker/MWetris) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
======
- [About](#about)
- [Requirements](#requirements)
  - [Dependencies](#dependencies)
  - [Building](#building)
- [Tests](#tests)
- [Simulate ai](#simulate-ai)
- [Things to fix](#things-to-fix)
- [License](#license)

## About
The project MWetris is a Tetris clone with network capabilities.

## Requirements
This project is easiest used with vcpkg and CMake. Visual Studio 2022 or GCC version >= 12.0.

```
git clone https://github.com/microsoft/vcpkg.git
```
Run bootstrap-vcpkg script belonging to the platform you are on.
Store the path to vcpkg in the enviromental variable VCPKG_ROOT.

## Simulate ai
You can customize tetris.json, and change the ai used by the game.

To try out different ai value functions change the ai in the json file and start the game to see the result. Or try the TetrisEngine project, optional in the cmake file.

Print help.
```
TetrisEngine --help
```

Start the game and print each move by the ai with 500 ms delay.
```
TetrisEngine --delay 500
```

Simulate a ai game using a custom value function and showing the end result:
```
TetrisEngine -a "-2*rowHoles - 5*columnHoles - 1*rowSumHeight / (1 + rowHoles) - 2*blockMeanHeight"
```

## Things to fix

- [] GameRules should be performed on the server with game time to make all players in sync. Will simplfy game logic. Current logic is a mess.
- [] Fix game board graphic to look nicer. Avoid using texture atlas because of ugly lines between squares.
- [] Make all unit tests to be run on codecov.
- [] Add file for theme settings for ImGui. To make the interface nicer.
- [] Make the network code compilable without graphic (make it possible to compile in raspberry pi).
- [] Add github action to add git tag version and create releases for windows and linux.
- [] Overuse of std::varient and to big objects (e.g. > 250 byte size). Maybe use shared_ptr or minimize the size of the objects.
- [] Maybe remove Calculator dependency in TetrisEngine and use a more simple way and faster way to calculate the value function.

## License
The project is under the MIT license (see LICENSE).
