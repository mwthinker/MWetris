project(App
	DESCRIPTION
		"A C++ Tetris Game"
	VERSION
		${PROJECT_VERSION}
	LANGUAGES
		CXX
)

set(SOURCES
	src/logger.cpp
	src/logger.h
	src/main.cpp
	src/mainwindow.cpp
	src/mainwindow.h

	CPack.cmake
	vcpkg.json
)

if (MSVC)
	configure_file(properties.rc ${CMAKE_CURRENT_BINARY_DIR}/properties.rc)
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS On) 

source_group(TREE
	${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${SOURCES}
)

find_package(argparse CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

# Copy data to build folder.
file(COPY ${MWetrisData_SOURCE_DIR}/package DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${MWetrisData_SOURCE_DIR}/images DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${MWetrisData_SOURCE_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${SDL_GameControllerDB_SOURCE_DIR}/gamecontrollerdb.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if (MSVC)
	add_executable(App
		${SOURCES}
		${MWetrisData_SOURCE_DIR}/package/mwetris_icon.rc
		${CMAKE_CURRENT_BINARY_DIR}/properties.rc
	)
else ()
	add_executable(App
		${SOURCES}
	)
endif ()

option(CODE_COVERAGE "Enable coverage reporting" OFF)

target_include_directories(App
	PRIVATE
		src
)

if (MSVC)
	target_compile_options(App
		PUBLIC
			/W3 /WX /permissive-
			/MP
	)
else ()
	target_compile_options(App
		PRIVATE
			-Wall -pedantic -Wcast-align -Woverloaded-virtual -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function
	)
endif ()

target_link_libraries(App
	PRIVATE
		argparse::argparse
		fmt::fmt
		MWetris::App_Lib
)

target_compile_definitions(App
	PRIVATE
		GIT_VERSION="${GIT_HASH}"
		GIT_DATE="${GIT_DATE}"
		PROJECT_VERSION="${PROJECT_VERSION}"
		VCPKG_HASH="${VCPKG_HASH}"
		VCPKG_DATE="${VCPKG_DATE}"
)

set_target_properties(App
	PROPERTIES
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO

		OUTPUT_NAME MWetris
)

include(${CMAKE_CURRENT_SOURCE_DIR}/CPack.cmake)
