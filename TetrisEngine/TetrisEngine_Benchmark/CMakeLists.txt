project(TetrisEngine_Benchmark
	DESCRIPTION
		"Benchmark the project TetrisEngine using google benchmark"
	LANGUAGES
		CXX
)

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4275") # non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
endif ()

find_package(benchmark CONFIG)
if (benchmark_FOUND)
	enable_testing()
		
	add_executable(TetrisEngine_Benchmark
		src/testspeed.cpp
	)

	target_link_libraries(TetrisEngine_Benchmark
		PRIVATE
			TetrisEngineLib
			benchmark::benchmark benchmark::benchmark_main
	)

	set_target_properties(TetrisEngine_Benchmark
		PROPERTIES
			CXX_STANDARD 20
			CXX_STANDARD_REQUIRED YES
			CXX_EXTENSIONS NO
	)
else ()
	message(WARNING "benchmark not found, TetrisEngine_Benchmark not created")
endif ()
