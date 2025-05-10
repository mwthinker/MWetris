include(FetchContent)
# Load external github projects
FetchContent_Declare(SDL_GameControllerDB
	GIT_REPOSITORY
		https://github.com/gabomdq/SDL_GameControllerDB.git
	GIT_TAG
		8834de54ffb8e87c4b5af350dbb16eeffc1158e1
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
		3eac3a08c43680cd8a5cef32001a75766126ef99
)
FetchContent_MakeAvailable(IconFontCppHeaders)
FetchContent_GetProperties(IconFontCppHeaders
	SOURCE_DIR
		IconFontCppHeaders_SOURCE_DIR
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

FetchContent_Declare(TomlCppGenerator
	GIT_REPOSITORY
		https://github.com/mwthinker/TomlCppGenerator.git
	GIT_TAG
		507fa0bf27c93d654e31e41eb274d218170f4686
)
FetchContent_MakeAvailable(TomlCppGenerator)
FetchContent_GetProperties(TomlCppGenerator
	SOURCE_DIR
		TomlCppGenerator_SOURCE_DIR
)
include(${TomlCppGenerator_SOURCE_DIR}/toml_generate_cpp.cmake)

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
