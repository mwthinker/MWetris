cmake_minimum_required(VERSION 3.11...3.14)
# 3.11 support for FetchContent
# 3.14 support for Visual Studio 2019

set(VCPKG_ROOT $ENV{VCPKG_ROOT})
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
		CACHE STRING ""
	)
	message(STATUS "Uses VCPKG CMAKE_TOOLCHAIN_FILE")
endif()

project(MWetris
	DESCRIPTION
		"A C++ wrapper of SDL2 and OpenGl"
	LANGUAGES
		CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4275") # non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251") # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
endif ()

set(SOURCES
	src/graphic/boardbatch.h
	src/graphic/boardshader.cpp
	src/graphic/boardshader.h
	src/graphic/drawblock.cpp
	src/graphic/drawblock.h
	src/graphic/drawrow.cpp
	src/graphic/drawrow.h
	src/graphic/drawtext.cpp
	src/graphic/drawtext.h
	src/graphic/gamecomponent.cpp
	src/graphic/gamecomponent.h
	src/graphic/gamegraphic.cpp
	src/graphic/gamegraphic.h	

	src/game/actionhandler.cpp
	src/game/actionhandler.h
	src/game/computer.cpp
	src/game/computer.h
	src/game/device.h
	src/game/gamerules.cpp
	src/game/gamerules.h
	src/game/input.h
	src/game/localgame.cpp
	src/game/localgame.h
	src/game/localplayer.cpp
	src/game/localplayer.h
	src/game/network.cpp
	src/game/network.h
	src/game/player.cpp
	src/game/playerdata.h
	src/game/player.h
	src/game/protocol.cpp
	src/game/protocol.h
	src/game/remoteplayer.cpp
	src/game/remoteplayer.h
	src/game/tetrisgame.cpp
	src/game/tetrisgameevent.h
	src/game/tetrisgame.h

	src/ui/imguiextra.cpp
	src/ui/imguiextra.h
	src/ui/imguishader.cpp
	src/ui/imguishader.h
	src/ui/imguiwindow.cpp
	src/ui/imguiwindow.h
	src/ui/tetriswindow.cpp
	src/ui/tetriswindow.h

	src/logger.h
	src/types.h
	src/taskqueue.h
	src/tetrisdata.cpp
	src/tetrisdata.h
	src/main.cpp
)

set_property(GLOBAL PROPERTY USE_FOLDERS On) 
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MWetris)

source_group(TREE
	${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${SOURCES}
)

add_subdirectory(TetrisEngine)
add_subdirectory(Protocol)

find_package(Threads REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

include(FetchContent)
# Load external github projects
FetchContent_Declare(CppSdl2
	GIT_REPOSITORY
		https://github.com/mwthinker/CppSdl2.git
)
FetchContent_MakeAvailable(CppSdl2)

FetchContent_Declare(CppProtoNetwork
	GIT_REPOSITORY
		https://github.com/mwthinker/CppProtoNetwork.git
)
FetchContent_MakeAvailable(CppProtoNetwork)

FetchContent_Declare(Signal
	GIT_REPOSITORY
		https://github.com/mwthinker/Signal.git
)
FetchContent_MakeAvailable(Signal)

# Load data.
FetchContent_Declare(MWetrisData
	GIT_REPOSITORY
		https://github.com/mwthinker/MWetrisData.git
)
FetchContent_MakeAvailable(MWetrisData)
FetchContent_GetProperties(MWetrisData
	SOURCE_DIR
		MWetrisData_SOURCE_DIR
)

set(ExternalDependencies
	CppSdl2
	CppProtoNetwork
	Calculator
)
if (CppSdl2Test)
	set(ExternalDependencieTests "CppSdl2Test")
endif ()
if (CalculatorTest)
	set(ExternalDependencieTests "${ExternalDependencieTests};CalculatorTest")
endif ()
if (CppProtoNetworkTest)
	set(ExternalDependencieTests "${ExternalDependencieTests};CppProtoNetworkTest")
endif ()

set_target_properties(
	${ExternalDependencies}
	${ExternalDependencieTests}
	
	PROPERTIES FOLDER
		ExternalDependencies
)

# Copy data to build folder.
file(COPY ${MWetrisData_SOURCE_DIR}/package DESTINATION ${CMAKE_CURRENT_BINARY_DIR}) 
file(COPY ${MWetrisData_SOURCE_DIR}/images DESTINATION ${CMAKE_CURRENT_BINARY_DIR}) 
file(COPY ${MWetrisData_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR}) 
file(COPY data/. DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(MWetris ${SOURCES})

if (MSVC)
	target_compile_definitions(MWetris
		PUBLIC
			_CRT_SECURE_NO_WARNINGS
	)
endif ()

target_link_libraries(MWetris
	PRIVATE
		glm
		Threads::Threads
		TetrisEngineLib
		nlohmann_json
		nlohmann_json::nlohmann_json		
		Protocol
		Signal
		${ExternalDependencies}
)
