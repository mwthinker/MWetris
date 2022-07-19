project(TetrisEngine
	DESCRIPTION
		"A lib for tetris board rules and ai logic"
	LANGUAGES
		CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----- TetrisEngineLib -------------
set(SOURCES_LIB
	srcLib/ai.cpp
	srcLib/ai.h
	srcLib/block.cpp
	srcLib/block.h
	srcLib/helper.cpp
	srcLib/helper.h
	srcLib/random.h
	srcLib/tetrisboard.cpp
	srcLib/tetrisboard.h
)

if (MSVC)
	configure_file(properties.rc ${CMAKE_CURRENT_BINARY_DIR}/properties.rc)
endif ()

add_library(TetrisEngineLib ${SOURCES_LIB})

target_include_directories(TetrisEngineLib
	PUBLIC
		srcLib
)

include(FetchContent)
fetchcontent_declare(Calculator
	GIT_REPOSITORY
		https://github.com/mwthinker/Calculator.git
	GIT_TAG
		7d64fcaa0b10e8aa39c2779bd4fd162f6f9ea2a5 
)
FetchContent_MakeAvailable(Calculator)

target_compile_definitions(TetrisEngineLib
	PUBLIC
		NOMINMAX
)

target_link_libraries(TetrisEngineLib
	PUBLIC
		Calculator
)

message(STATUS "TetrisEngine_Test is available to add: -DTetrisEngine_Test=1")
option(TetrisEngine_Test "Add TetrisEngine_Test to project." OFF)
if (TetrisEngine_Test)
	add_subdirectory(TetrisEngine_Test)
endif ()

# ----- TetrisEngine -------------
set(SOURCES_MAIN
	src/flags.cpp
	src/flags.h
	src/flagsexception.h
	src/main.cpp
)

if (MSVC)
	add_executable(TetrisEngine 
		${SOURCES_MAIN}
		${CMAKE_CURRENT_BINARY_DIR}/properties.rc
	)
else ()
	add_executable(TetrisEngine
		${SOURCES_MAIN}
	)
endif ()
	
target_link_libraries(TetrisEngine
	PRIVATE
		TetrisEngineLib
		fmt::fmt-header-only
)

# Copy data to build folder.
file(COPY data/. DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS TetrisEngine
	EXPORT TetrisEngine
	DESTINATION .
	COMPONENT application
)
