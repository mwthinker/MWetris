include(FetchContent)
# Load external github projects
FetchContent_Declare(CppSdl2
	GIT_REPOSITORY
		https://github.com/mwthinker/CppSdl2.git
	GIT_TAG
		bb98e024fe356bf4b019f8f4e3af140bec66eb5f
	OVERRIDE_FIND_PACKAGE
)

fetchcontent_declare(SDL_GameControllerDB
	GIT_REPOSITORY
		https://github.com/gabomdq/SDL_GameControllerDB.git
	GIT_TAG
		da332bb484c2434269b9264f7dce3e6114227755
)
FetchContent_MakeAvailable(SDL_GameControllerDB)
FetchContent_GetProperties(SDL_GameControllerDB
	SOURCE_DIR
		SDL_GameControllerDB_SOURCE_DIR
)

FetchContent_Declare(CppProtoNetwork
	GIT_REPOSITORY
		https://github.com/mwthinker/CppProtoNetwork.git
	GIT_TAG
		13dfa51590b971c67fc23e58615af6ea39125109
	OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(Signal
	GIT_REPOSITORY
		https://github.com/mwthinker/Signal.git
	GIT_TAG
		0daa9f5fd3ef2368be3634751b192f59479d71c7
	OVERRIDE_FIND_PACKAGE
)

fetchcontent_declare(Calculator
	GIT_REPOSITORY
		https://github.com/mwthinker/Calculator.git
	GIT_TAG
		5135c415b9fab24ae2725c210c428b1c3f83637a
	OVERRIDE_FIND_PACKAGE
)

fetchcontent_declare(CmakeAuxiliary
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
