cmake_minimum_required(VERSION 4.2)
project(MWetris
	DESCRIPTION
		"A C++ Tetris Game project"
	VERSION
		${PROJECT_VERSION}
	LANGUAGES
		CXX
)

source_group(".github" FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/.github/copilot-instructions.md"
)
source_group(".github/workflows" FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/ci.yml"
)
source_group(".vscode" FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/.vscode/settings.json"
)
source_group("Docker" FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Docker/build-server"
	"${CMAKE_CURRENT_SOURCE_DIR}/Docker/docker-compose.yml"
	"${CMAKE_CURRENT_SOURCE_DIR}/Docker/Dockerfile"
)

set_property(DIRECTORY APPEND PROPERTY VS_SOLUTION_ITEMS
	"${CMAKE_CURRENT_SOURCE_DIR}/.gitattributes"
	"${CMAKE_CURRENT_SOURCE_DIR}/.github/copilot-instructions.md"
	"${CMAKE_CURRENT_SOURCE_DIR}/.github/workflows/ci.yml"
	"${CMAKE_CURRENT_SOURCE_DIR}/.gitignore"
	"${CMAKE_CURRENT_SOURCE_DIR}/.vscode/settings.json"
	"${CMAKE_CURRENT_SOURCE_DIR}/AGENTS.md"
	"${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt"
	"${CMAKE_CURRENT_SOURCE_DIR}/CMakePresets.json"
	"${CMAKE_CURRENT_SOURCE_DIR}/Docker/build-server"
	"${CMAKE_CURRENT_SOURCE_DIR}/Docker/docker-compose.yml"
	"${CMAKE_CURRENT_SOURCE_DIR}/Docker/Dockerfile"
	"${CMAKE_CURRENT_SOURCE_DIR}/ExternalFetchContent.cmake"
	"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
	"${CMAKE_CURRENT_SOURCE_DIR}/README.md"
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

add_subdirectory(Protocol)
add_subdirectory(Network_Lib)
add_subdirectory(GameServer)
add_subdirectory(TetrisEngine)

if (NOT GAME_SERVER_ONLY)
    add_subdirectory(App_Lib)
    add_subdirectory(App_Test)
    add_subdirectory(App)
endif()

option(CODE_COVERAGE "Enable coverage reporting" OFF)
