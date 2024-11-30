project(App_Test
	DESCRIPTION
		"Testing project using GTest"
	LANGUAGES
		CXX
)

find_package(GTest CONFIG REQUIRED)
enable_testing()

set(SOURCES_TEST
	src/game/devicemanagertest.cpp
	src/game/serializetest.cpp
	src/mwetristest.cpp
	src/network/gameroomtest.cpp
	src/network/networktest.cpp
	src/network/protobufmessagetest.cpp
	src/network/testutil.cpp
	src/network/testutil.h
	src/main.cpp

	CMakeLists.txt
)

set_property(GLOBAL PROPERTY USE_FOLDERS On)
source_group(TREE
	${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${SOURCES_TEST}
)

add_executable(App_Test
	${SOURCES_TEST}
)

target_link_libraries(App_Test
	PUBLIC
		GTest::gtest GTest::gmock
		MWetris::App_Lib
)

if (MSVC)
	target_compile_options(App_Test
		PRIVATE
			"/permissive-"
	)
endif ()

set_target_properties(App_Test
	PROPERTIES
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)

include(GoogleTest)
gtest_discover_tests(App_Test)
