project(TetrisEngine_Test
	DESCRIPTION
		"Test the project TetrisEngine using Catch2"
	LANGUAGES
		CXX
)

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4275") # non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
endif ()

find_package(Catch2 CONFIG)
if (Catch2_FOUND)
	enable_testing()
		
	include(Catch)

	add_executable(TetrisEngine_Test
		src/testtetrisengine.cpp
	)
	
	target_link_libraries(TetrisEngine_Test
		PRIVATE
			TetrisEngineLib
			Catch2::Catch2 Catch2::Catch2WithMain
	)

	set_target_properties(TetrisEngine_Test
		PROPERTIES
			CXX_STANDARD 20
			CXX_STANDARD_REQUIRED YES
			CXX_EXTENSIONS NO
	)

	catch_discover_tests(TetrisEngine_Test)
else ()
	message(WARNING "Catch2 not found, TetrisEngine_Test not created")
endif ()
