include(FetchContent)
# Load external github projects
FetchContent_Declare(CppSdl2
	GIT_REPOSITORY
		https://github.com/mwthinker/CppSdl2.git
	GIT_TAG
		82a55c4b9e1d9eea4cca88fba6bc8494d4d58968
	OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(SDL_GameControllerDB
	GIT_REPOSITORY
		https://github.com/gabomdq/SDL_GameControllerDB.git
	GIT_TAG
		99c8ec90ac72eac926aab3a6ce8488d50f0226e0
)
FetchContent_MakeAvailable(SDL_GameControllerDB)
FetchContent_GetProperties(SDL_GameControllerDB
	SOURCE_DIR
		SDL_GameControllerDB_SOURCE_DIR
)

FetchContent_Declare(Signal
	GIT_REPOSITORY
		https://github.com/mwthinker/Signal.git
	GIT_TAG
		7f3f8c456cfdb90ea014b335f175e6576493d48b
	OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(Calculator
	GIT_REPOSITORY
		https://github.com/mwthinker/Calculator.git
	GIT_TAG
		527fce2eb6ece1f653abfbc4431916addeedde5c
	OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(CmakeAuxiliary
	GIT_REPOSITORY
		https://gist.github.com/mwthinker/e0539b200e3f0ac6da545af4843a6ff6
	GIT_TAG
		0f0aaa9201a5a4c11749cd5adfc496a3960014de
)
FetchContent_MakeAvailable(CmakeAuxiliary)
FetchContent_GetProperties(CmakeAuxiliary
	SOURCE_DIR
		CmakeAuxiliary_SOURCE_DIR
)
include(${CmakeAuxiliary_SOURCE_DIR}/auxiliary.cmake)

# Load data.
FetchContent_Declare(MWetrisData
	GIT_REPOSITORY
		https://github.com/mwthinker/MWetrisData.git
	GIT_TAG
		bb892f628dcb0cde99ca30f7b57e3f1b82e5fd2c
)
FetchContent_MakeAvailable(MWetrisData)
FetchContent_GetProperties(MWetrisData
	SOURCE_DIR
		MWetrisData_SOURCE_DIR
)
