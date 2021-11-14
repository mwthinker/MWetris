MWetris - Simple and fun tetris game
======
- [About](#about)
- [Requirements](#requirements)
  - [Dependencies](#dependencies)
  - [Building](#building)
- [Tests](#tests)
- [Simulate ai](#simulate-ai)
- [Running the game](#running-the-game)
- [License](#license)

## About
The project MWetris is a Tetris clone with network capabilities. You 
can play up to 4 human players on one computer and up to 4 computer opponents.

## Requirements
This project is easiest used with vcpkg and CMake. Visual Studio 2022 or GCC version >= 10.0.

```
git clone https://github.com/microsoft/vcpkg.git
```
Run bootstrap-vcpkg script belonging to the platform you are on.
Store the path to vcpkg in the enviromental variable VCPKG_ROOT.

### Command line
Run the executable
```
MWetrisTest --help # See help
MWetrisTest *      # Run Everytning (MWetrisTest.exe in windows)
```

### Visual Studio
In order for Visual studio to add tests to the Visual Studio Explorer one may use [Test Adapter for Catch2](https://marketplace.visualstudio.com/items?itemName=JohnnyHendriks.ext01). Git project located at https://github.com/JohnnyHendriks/TestAdapter_Catch2.git. One must add "Test Settings file" in "Test" menu. Use Minimal.runsettings.

## Simulate ai
You can customize tetris.json, and change the ai used by the game.

To try out different ai value functions change the ai in the json file and start the game to see the result. Or try the TetrisEngineTest project, optional in the cmake file.

Print help.
```
TetrisEngineTest --help
```

Start the game and print each move by the ai with 500 ms delay.
```
TetrisEngineTest --delay 500
```

Simulate a ai game using a custom value function and showing the end result:
```
TetrisEngineTest -a "-2*rowHoles - 5*columnHoles - 1*rowSumHeight / (1 + rowHoles) - 2*blockMeanHeight"
```

## License
The project is under the MIT license (see LICENSE.txt).
