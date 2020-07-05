project(TetrisEngine
	DESCRIPTION
		"A lib for tetris rules and ai logic"
	LANGUAGES
		CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----- TetrisEngineLib -------------
set(SOURCES_LIB
	srcLib/ai.cpp
	srcLib/ai.h
	srcLib/block.cpp
	srcLib/block.h
	srcLib/random.h
	srcLib/rawtetrisboard.cpp
	srcLib/rawtetrisboard.h
	srcLib/square.h
	srcLib/tetrisboard.cpp
	srcLib/tetrisboard.h
)

add_library(TetrisEngineLib ${SOURCES_LIB})

target_include_directories(TetrisEngineLib
	PUBLIC
		srcLib
)

include(FetchContent)
fetchcontent_declare(Signal
	GIT_REPOSITORY
		https://github.com/mwthinker/Signal.git
)
FetchContent_MakeAvailable(Signal)

fetchcontent_declare(Calculator
	GIT_REPOSITORY
		https://github.com/mwthinker/Calculator.git
)
FetchContent_MakeAvailable(Calculator)

target_compile_definitions(TetrisEngineLib
	PUBLIC
		NOMINMAX
)

target_link_libraries(TetrisEngineLib
	PUBLIC
		Calculator

		Signal
		EnTT::EnTT
)

message(STATUS "TetrisEngine_Test is available to add: -DTetrisEngine_Test=1")
option(TetrisEngine_Test "Add TetrisEngine_Test to project." OFF)
if (TetrisEngine_Test)
	add_subdirectory(TetrisEngine_Test)
endif (TetrisEngine_Test)

# ----- TetrisEngine -------------
set(SOURCES_MAIN
	src/flags.cpp
	src/flags.h
	src/flagsexception.h
	src/main.cpp
)

add_executable(TetrisEngine ${SOURCES_MAIN})
	
target_link_libraries(TetrisEngine
	PRIVATE
		TetrisEngineLib
		fmt::fmt fmt::fmt-header-only
)

# Copy data to build folder.
file(COPY data/. DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS TetrisEngine
	EXPORT TetrisEngine
	DESTINATION .
	COMPONENT application
)
