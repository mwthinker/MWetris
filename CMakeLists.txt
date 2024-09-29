cmake_minimum_required(VERSION 3.25)
project(MWetris
	DESCRIPTION
		"A C++ Tetris Game project"
	VERSION
		${PROJECT_VERSION}
	LANGUAGES
		CXX
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT App)

option(GAME_SERVER_ONLY "Game Server only" OFF)
include(ExternalFetchContent.cmake)

extract_git_hash(
	${CMAKE_CURRENT_SOURCE_DIR}
	GIT_HASH
	GIT_DATE
)
extract_git_hash(
	$ENV{VCPKG_ROOT}
	VCPKG_HASH
	VCPKG_DATE
)
create_guid_from_source(
	${CMAKE_CURRENT_SOURCE_DIR}
	GUID
)

message(STATUS "VCPKG_HASH: ${VCPKG_HASH} ${VCPKG_DATE}")
message(STATUS "GIT_HASH: ${GIT_HASH} ${GIT_DATE}")
message(STATUS "CMAKE_PROJECT_VERSION: ${CMAKE_PROJECT_VERSION}")
message(STATUS "GUID: ${GUID}")

if (GAME_SERVER_ONLY)
	set(ExternalDependencies
		Calculator
	)
else ()
	set(ExternalDependencies
		CppSdl2
		Calculator
		ImGui
	)
endif ()

if (CppSdl2_Test AND NOT GAME_SERVER_ONLY)
	set(ExternalDependencieTests "${ExternalDependencieTests};CppSdl2_Test")
endif ()
if (CppSdl2_Example AND NOT GAME_SERVER_ONLY)
	set(ExternalDependencieTests "${ExternalDependencieTests};CppSdl2_Example")
endif ()
if (Calculator_Test)
	set(ExternalDependencieTests "${ExternalDependencieTests};Calculator_Test")
endif ()
if (Signal_Test)
	set(ExternalDependencieTests "${ExternalDependencieTests};Signal_Test")
endif ()

add_subdirectory(Protocol)
add_subdirectory(Network_Lib)
add_subdirectory(GameServer)
add_subdirectory(TetrisEngine)

if (NOT GAME_SERVER_ONLY)
	add_subdirectory(App_Lib)
	add_subdirectory(App_Test)
	add_subdirectory(App)
endif ()

set_target_properties(
	${ExternalDependencies}
	${ExternalDependencieTests}
	
	PROPERTIES FOLDER
		ExternalDependencies
)

option(CODE_COVERAGE "Enable coverage reporting" OFF)
