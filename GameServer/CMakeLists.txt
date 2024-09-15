project(GameServer
	DESCRIPTION
		"The MWetris GameServer"
	VERSION
		${PROJECT_VERSION}
	LANGUAGES
		CXX
)

find_package(Threads REQUIRED)
find_package(argparse CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(GameServer
	src/main.cpp
)

target_link_libraries(GameServer
	PRIVATE
		argparse::argparse
		Network_Lib
		fmt::fmt
		spdlog::spdlog_header_only
)

target_compile_definitions(GameServer
	PUBLIC
		PROJECT_VERSION="${CMAKE_PROJECT_VERSION}"
)

if (MSVC)
	target_compile_options(GameServer
		PRIVATE
			"/permissive-"
			"/wd4251" # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
	)
endif ()

set_target_properties(GameServer
	PROPERTIES
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)
