project(TetrisEngine_Test
	DESCRIPTION
		"Test the project TetrisEngine using Catch2"
	LANGUAGES
		CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4275") # non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
endif ()

find_package(Catch2 CONFIG)
if (Catch2_FOUND)
	add_executable(TetrisEngine_Test
		src/testspeed.cpp
		src/testtetrisengine.cpp
	)
	
	add_compile_definitions(CATCH_CONFIG_ENABLE_BENCHMARKING)

	target_link_libraries(TetrisEngine_Test
		PRIVATE
			TetrisEngineLib
			Catch2::Catch2 Catch2::Catch2WithMain
	)
else (Catch2_FOUND)
	message(WARNING "Catch2 not found, TetrisEngine_Test not created")
endif (Catch2_FOUND)
