cmake_minimum_required(VERSION 3.29)
project(MWetrisProject
	DESCRIPTION
		"A C++ Tetris Game project"
	VERSION
		${PROJECT_VERSION}
	LANGUAGES
		CXX
)

#set_property(GLOBAL PROPERTY USE_FOLDERS On) 
#set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MWetris)

#source_group(TREE
#	${CMAKE_CURRENT_SOURCE_DIR}
#	FILES
#		${SOURCES}
#)

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

# Copy data to build folder.
file(COPY ${MWetrisData_SOURCE_DIR}/package DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${MWetrisData_SOURCE_DIR}/images DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${MWetrisData_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY data/. DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${SDL_GameControllerDB_SOURCE_DIR}/gamecontrollerdb.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

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
	add_subdirectory(MWetris_Lib)
	add_subdirectory(MWetris_Test)
	add_subdirectory(MWetris)
endif ()

set_target_properties(
	${ExternalDependencies}
	${ExternalDependencieTests}
	
	PROPERTIES FOLDER
		ExternalDependencies
)

option(CODE_COVERAGE "Enable coverage reporting" OFF)
