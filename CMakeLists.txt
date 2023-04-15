cmake_minimum_required(VERSION 3.24)
project(MWetris
	DESCRIPTION
		"A C++ Tetris Game"
	VERSION
		0.1.0
	LANGUAGES
		CXX
)

set(SOURCES
	src/logger.cpp
	src/logger.h
	src/main.cpp

	ExternalFetchContent.cmake
	CPack.cmake
	vcpkg.json
)

if (MSVC)
	configure_file(properties.rc ${CMAKE_CURRENT_BINARY_DIR}/properties.rc)
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS On) 
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MWetris)

source_group(TREE
	${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${SOURCES}
)

include(ExternalFetchContent.cmake)

find_package(Threads REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

find_package(CppSdl2 REQUIRED)
find_package(CppProtoNetwork REQUIRED)
find_package(Signal REQUIRED)
find_package(Calculator REQUIRED)

extract_git_hash()
create_guid()

message(STATUS "GIT_HASH: ${GIT_HASH}")
message(STATUS "PROJECT_VERSION: ${PROJECT_VERSION}")
message(STATUS "GUID: ${GUID}")

set(ExternalDependencies
	CppSdl2
	CppProtoNetwork
	Calculator
	ImGui
)
if (CppSdl2_Test)
	set(ExternalDependencieTests "${ExternalDependencieTests};CppSdl2_Test")
endif ()
if (CppSdl2_Example)
	set(ExternalDependencieTests "${ExternalDependencieTests};CppSdl2_Example")
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

add_subdirectory(TetrisEngine)
add_subdirectory(Protocol)
add_subdirectory(MWetris_Lib)
add_subdirectory(MWetris_Test)

# Copy data to build folder.
file(COPY ${MWetrisData_SOURCE_DIR}/package DESTINATION ${CMAKE_CURRENT_BINARY_DIR}) 
file(COPY ${MWetrisData_SOURCE_DIR}/images DESTINATION ${CMAKE_CURRENT_BINARY_DIR}) 
file(COPY ${MWetrisData_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR}) 
file(COPY data/. DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if (MSVC)
	add_executable(MWetris
		${SOURCES}
		${MWetrisData_SOURCE_DIR}/package/mwetris_icon.rc
		${CMAKE_CURRENT_BINARY_DIR}/properties.rc
	)
else ()
	add_executable(MWetris
		${SOURCES}
	)
endif ()


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
			/W3 /WX /permissive-
			/MP
	)
else ()
	target_compile_options(MWetris
		PRIVATE
			-Wall -pedantic -Wcast-align -Woverloaded-virtual -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function
	)
endif ()

target_link_libraries(MWetris
	PRIVATE
		MWetris_Lib
)

target_compile_definitions(MWetris
	PRIVATE
		GIT_VERSION="${GIT_HASH}"
		PROJECT_VERSION="${PROJECT_VERSION}"
)

set_target_properties(MWetris
	PROPERTIES
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)

include(${CMAKE_CURRENT_SOURCE_DIR}/CPack.cmake)
