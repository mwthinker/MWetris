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

add_subdirectory(Protocol)
add_subdirectory(Network_Lib)
add_subdirectory(GameServer)
add_subdirectory(TetrisEngine)

if (GAME_SERVER_ONLY)
	set_target_properties(
		Calculator
		
		PROPERTIES FOLDER
			ExternalDependencies
	)

	if (Calculator_Test)
		set_target_properties(Calculator_Test PROPERTIES FOLDER ExternalDependencies)
	endif ()
	if (Signal_Test)
		set_target_properties(Signal_Test PROPERTIES FOLDER ExternalDependencies)
	endif ()

else()
	add_subdirectory(App_Lib)
	add_subdirectory(App_Test)
	add_subdirectory(App)

	set_target_properties(
		CppSdl2
		Calculator
		ImGui
		
		PROPERTIES FOLDER
			ExternalDependencies
	)

	if (CppSdl2_Test)
		set_target_properties(CppSdl2_Test PROPERTIES FOLDER ExternalDependencies)
	endif ()
	if (CppSdl2_Example)
		set_target_properties(CppSdl2_Example PROPERTIES FOLDER ExternalDependencies)
	endif ()
	if (Calculator_Test)
		set_target_properties(Calculator_Test PROPERTIES FOLDER ExternalDependencies)
	endif ()
	if (Signal_Test)
		set_target_properties(Signal_Test PROPERTIES FOLDER ExternalDependencies)
	endif ()

endif ()

option(CODE_COVERAGE "Enable coverage reporting" OFF)
