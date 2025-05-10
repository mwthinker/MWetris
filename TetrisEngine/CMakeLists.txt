project(TetrisEngine
	DESCRIPTION
		"A lib for tetris board rules and ai logic"
	VERSION
		${PROJECT_VERSION}
	LANGUAGES
		CXX
)

# ----- TetrisEngine_Lib -------------
set(SOURCES_LIB
	srcLib/tetris/ai.cpp
	srcLib/tetris/ai.h
	srcLib/tetris/block.cpp
	srcLib/tetris/block.h
	srcLib/tetris/helper.cpp
	srcLib/tetris/helper.h
	srcLib/tetris/random.h
	srcLib/tetris/tetrisboard.cpp
	srcLib/tetris/tetrisboard.h
)

if (MSVC)
	configure_file(properties.rc ${CMAKE_CURRENT_BINARY_DIR}/properties.rc)
endif ()

add_library(TetrisEngine_Lib ${SOURCES_LIB})
add_library(MWetris::TetrisEngine_Lib ALIAS TetrisEngine_Lib)

target_include_directories(TetrisEngine_Lib
	PUBLIC
		srcLib
)

find_package(Calculator CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

if (CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(TetrisEngine_Lib PUBLIC --coverage)
	target_link_options(TetrisEngine_Lib PUBLIC --coverage)
endif ()

target_compile_definitions(TetrisEngine_Lib
	PUBLIC
		NOMINMAX
)

target_link_libraries(TetrisEngine_Lib
	PUBLIC
		Calculator::Calculator
)

set_target_properties(TetrisEngine_Lib
	PROPERTIES
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)

message(STATUS "TetrisEngine_Test is available to add: -DTetrisEngine_Test=1")
option(TetrisEngine_Test "Add TetrisEngine_Test to project." OFF)
if (TetrisEngine_Test)
	add_subdirectory(TetrisEngine_Test)
endif ()

message(STATUS "TetrisEngine_Benchmark is available to add: -DTetrisEngine_Benchmark=1")
option(TetrisEngine_Benchmark "Add TetrisEngine_Benchmark to project." OFF)
if (TetrisEngine_Benchmark)
	add_subdirectory(TetrisEngine_Benchmark)
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
		fmt::fmt
		MWetris::TetrisEngine_Lib
)

set_target_properties(TetrisEngine
	PROPERTIES
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)

# Copy data to build folder.
file(COPY data/. DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

install(TARGETS TetrisEngine
	EXPORT TetrisEngine
	DESTINATION .
	COMPONENT application
)
