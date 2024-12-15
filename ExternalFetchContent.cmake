include(FetchContent)
# Load external github projects
if (NOT GAME_SERVER_ONLY)
	FetchContent_Declare(CppSdl2
		GIT_REPOSITORY
			https://github.com/mwthinker/CppSdl2.git
		GIT_TAG
			4fe016d31ee5b83f15ac261c8938038b0b57c5cd
		OVERRIDE_FIND_PACKAGE
	)
endif()

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

FetchContent_Declare(IconFontCppHeaders
	GIT_REPOSITORY
		https://github.com/juliettef/IconFontCppHeaders.git
	GIT_TAG
		173c9b99be9a9980d2eeaf4b627d8c14b344bc68
)
FetchContent_MakeAvailable(IconFontCppHeaders)
FetchContent_GetProperties(IconFontCppHeaders
	SOURCE_DIR
		IconFontCppHeaders_SOURCE_DIR
)

FetchContent_Declare(Signal
	GIT_REPOSITORY
		https://github.com/mwthinker/Signal.git
	GIT_TAG
		37e28bbbfcb7a6f9794af37c862f781f0df543a0
	OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(Calculator
	GIT_REPOSITORY
		https://github.com/mwthinker/Calculator.git
	GIT_TAG
		97194c77a446e975233fca1f14138cd103f2f463
	OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(CMakeHelper
	GIT_REPOSITORY
		https://github.com/mwthinker/CMakeHelper.git
	GIT_TAG
		a321588de57895a85e642810b6eedc5cbf89c0ea
)
FetchContent_MakeAvailable(CMakeHelper)
FetchContent_GetProperties(CMakeHelper
	SOURCE_DIR
		CMakeHelper_SOURCE_DIR
)
include(${CMakeHelper_SOURCE_DIR}/auxiliary.cmake)

# Load data.
FetchContent_Declare(MWetrisData
	GIT_REPOSITORY
		https://github.com/mwthinker/MWetrisData.git
	GIT_TAG
		a68929105254ec3fab8ebe8a4279b9763b83d007
)
FetchContent_MakeAvailable(MWetrisData)
FetchContent_GetProperties(MWetrisData
	SOURCE_DIR
		MWetrisData_SOURCE_DIR
)
