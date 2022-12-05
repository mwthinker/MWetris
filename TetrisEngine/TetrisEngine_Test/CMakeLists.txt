project(TetrisEngine_Test
	DESCRIPTION
		"Test the project TetrisEngine using Catch2"
	LANGUAGES
		CXX
)

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4275") # non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
endif ()

find_package(GTest CONFIG)
if (GTest_FOUND)
	enable_testing()
		
	add_executable(TetrisEngine_Test
		src/testtetrisengine.cpp
	)
	
	target_link_libraries(TetrisEngine_Test
		PRIVATE
			TetrisEngineLib
			GTest::gtest GTest::gtest_main
	)

	set_target_properties(TetrisEngine_Test
		PROPERTIES
			CXX_STANDARD 20
			CXX_STANDARD_REQUIRED YES
			CXX_EXTENSIONS NO
	)

	include(GoogleTest)
	gtest_discover_tests(TetrisEngine_Test)
else ()
	message(WARNING "GTest not found, TetrisEngine_Test not created")
endif ()
