project(MWetris_Test
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

	CMakeLists.txt
)

set_property(GLOBAL PROPERTY USE_FOLDERS On)
source_group(TREE
	${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${SOURCES_TEST}
)

add_executable(MWetris_Test
	${SOURCES_TEST}
)

target_link_libraries(MWetris_Test
	PUBLIC
		MWetris_Lib
		GTest::gtest GTest::gtest_main # Test explorer on Visual Studio 2022 will not find test if "GTest::gmock_main GTest::gmock" is added?
)

if (MSVC)
	target_compile_options(MWetris_Test
		PRIVATE
			"/permissive-"
	)
endif ()

set_target_properties(MWetris_Test
	PROPERTIES
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)

include(GoogleTest)
gtest_discover_tests(MWetris_Test)
