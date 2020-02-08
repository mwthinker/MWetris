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
	src/game/tetrisgame.cpp
	src/game/tetrisgameevent.h
	src/game/tetrisgame.h

	src/ui/imguiextra.cpp
	src/ui/imguiextra.h
	src/ui/tetriswindow.cpp
	src/ui/tetriswindow.h

	src/logger.h
	src/types.h
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
	GIT_TAG
		4ddb8f9ab857ca039a3d671c77e608d7c5895ceb
)
FetchContent_MakeAvailable(CppSdl2)

FetchContent_Declare(CppProtoNetwork
	GIT_REPOSITORY
		https://github.com/mwthinker/CppProtoNetwork.git
	GIT_TAG
		6211440420ef56402e1944c4639ac355141ef70d
)
FetchContent_MakeAvailable(CppProtoNetwork)

FetchContent_Declare(Signal
	GIT_REPOSITORY
		https://github.com/mwthinker/Signal.git
	GIT_TAG
		76054e3797c62247910adb83e05f32cc47885faa
)
FetchContent_MakeAvailable(Signal)

# Load data.
FetchContent_Declare(MWetrisData
	GIT_REPOSITORY
		https://github.com/mwthinker/MWetrisData.git
	GIT_TAG
		d9bfd28c799b71e6048b51ae7f591f4853b7e887
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

if (MSVC)
	target_compile_definitions(MWetris
		PUBLIC
			_CRT_SECURE_NO_WARNINGS
	)
endif ()

target_link_libraries(MWetris
	PRIVATE
		TetrisEngineLib
		nlohmann_json
		nlohmann_json::nlohmann_json
		Protocol
		Signal
		${ExternalDependencies}
)
