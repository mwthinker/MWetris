cmake_minimum_required(VERSION 3.14)
# 3.14 support for Visual Studio 2019

set(VCPKG_ROOT $ENV{VCPKG_ROOT})
if (DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
		CACHE STRING ""
	)
	message(STATUS "Uses VCPKG CMAKE_TOOLCHAIN_FILE")
endif()

project(MWetris
	DESCRIPTION
		"A C++ clone of Tetris"
	VERSION
		0.1.0
	LANGUAGES
		CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4275") # non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251") # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
endif ()

set(SOURCES
	src/graphic/drawboard.cpp
	src/graphic/drawboard.h
	src/graphic/drawrow.cpp
	src/graphic/drawrow.h
	src/graphic/gamecomponent.cpp
	src/graphic/gamecomponent.h
	src/graphic/gamegraphic.cpp
	src/graphic/gamegraphic.h
	src/graphic/graphic.cpp
	src/graphic/graphic.h

	src/game/actionhandler.cpp
	src/game/actionhandler.h
	src/game/computer.cpp
	src/game/computer.h
	src/game/device.h
	src/game/gamerules.cpp
	src/game/gamerules.h
	src/game/input.h
	src/game/keyboard.cpp
	src/game/keyboard.h
	src/game/localgame.cpp
	src/game/localgame.h
	src/game/localplayer.cpp
	src/game/localplayer.h
	src/game/network.cpp
	src/game/network.h
	src/game/player.cpp
	src/game/playerdata.h
	src/game/player.h
	src/game/remoteplayer.cpp
	src/game/remoteplayer.h
	src/game/sdldevice.h
	src/game/tetrisgame.cpp
	src/game/tetrisgameevent.h
	src/game/tetrisgame.h
	
	src/ui/scene/custom.cpp
	src/ui/scene/custom.h
	src/ui/scene/highscore.cpp
	src/ui/scene/highscore.h
	src/ui/scene/settings.cpp
	src/ui/scene/settings.h
	src/ui/scene/event.h
	src/ui/scene/play.cpp
	src/ui/scene/play.h
	src/ui/scene/menu.cpp
	src/ui/scene/menu.h
	src/ui/scene/network.cpp
	src/ui/scene/network.h
	src/ui/scene/scene.h
	src/ui/scene/statemachine.cpp
	src/ui/scene/statemachine.h

	src/ui/imguiextra.cpp
	src/ui/imguiextra.h
	src/ui/tetriswindow.cpp
	src/ui/tetriswindow.h
	
	src/logger.cpp
	src/logger.h
	src/types.h
	src/tetrisdata.cpp
	src/tetrisdata.h
	src/main.cpp

	ExternalFetchContent.cmake
)

set_property(GLOBAL PROPERTY USE_FOLDERS On) 
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MWetris)

source_group(TREE
	${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${SOURCES}
)

find_package(Threads REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)

add_subdirectory(TetrisEngine)
add_subdirectory(Protocol)

include(ExternalFetchContent.cmake)

extract_git_hash()
create_guid()

message(STATUS "GIT_HASH: ${GIT_HASH}")
message(STATUS "PROJECT_VERSION: ${PROJECT_VERSION}")
message(STATUS "GUID: ${GUID}")

set(ExternalDependencies
	CppSdl2
	CppProtoNetwork
	Calculator
)
if (CppSdl2_Test)
	set(ExternalDependencieTests "CppSdl2_Test")
endif ()
if (CppSdl2_Example)
	set(ExternalDependencieTests "CppSdl2_Example")
endif ()
if (Calculator_Test)
	set(ExternalDependencieTests "${ExternalDependencieTests};Calculator_Test")
endif ()
if (CppProtoNetwork_Test)
	set(ExternalDependencieTests "${ExternalDependencieTests};CppProtoNetwork_Test")
endif ()
if (Signal_Test)
	set(ExternalDependencieTests "${ExternalDependencieTests};Signal_Test")
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

target_include_directories(MWetris
	PRIVATE
		src
)

if (MSVC)
	target_compile_definitions(MWetris
		PUBLIC
			_CRT_SECURE_NO_WARNINGS
			NOMINMAX
	)

	target_compile_options(MWetris
		PUBLIC
			"/permissive-"
			"/MP"
	)
endif ()

target_link_libraries(MWetris
	PRIVATE
		TetrisEngineLib
		nlohmann_json
		nlohmann_json::nlohmann_json
		Protocol
		Signal
		EnTT::EnTT
		${ExternalDependencies}
)

target_compile_definitions(MWetris
	PRIVATE
		GIT_VERSION="${GIT_HASH}"
		PROJECT_VERSION="${PROJECT_VERSION}"
)
