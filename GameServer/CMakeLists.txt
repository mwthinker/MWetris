project(GameServer
	DESCRIPTION
		"The MWetris GameServer"
	LANGUAGES
		CXX
)

find_package(fmt CONFIG REQUIRED)

add_executable(GameServer
	src/main.cpp
)

target_link_libraries(GameServer
	PRIVATE
		MWetris_Lib
		Protocol
		fmt::fmt-header-only
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
