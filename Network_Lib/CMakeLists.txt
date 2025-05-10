project(Network_Lib
	DESCRIPTION
		"Network library"
	LANGUAGES
		CXX
)

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4275") # non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251") # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
endif ()

set(SOURCES_LIB
	src/network/asio.h
	src/network/auxiliary.h
	src/network/client.h
	src/network/debugclient.cpp
	src/network/debugclient.h
	src/network/debugserver.cpp
	src/network/debugserver.h
	src/network/gameroom.cpp
	src/network/gameroom.h
	src/network/id.cpp
	src/network/id.h
	src/network/protobufmessage.cpp
	src/network/protobufmessage.h
	src/network/protobufmessagequeue.cpp
	src/network/protobufmessagequeue.h
	src/network/server.h
	src/network/servercore.cpp
	src/network/servercore.h
	src/network/tcpclient.cpp
	src/network/tcpclient.h
	src/network/tcpserver.cpp
	src/network/tcpserver.h
)

find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Signal CONFIG REQUIRED)

set_property(GLOBAL PROPERTY USE_FOLDERS On)
source_group(TREE
	${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${SOURCES_LIB}
)

add_library(Network_Lib ${SOURCES_LIB})
add_library(MWetris::Network_Lib ALIAS Network_Lib)

target_include_directories(Network_Lib
	PUBLIC
		src/
)


target_compile_definitions(Network_Lib
	PUBLIC
		# ASIO non-boost version.
		ASIO_STANDALONE
		ASIO_NO_DEPRECATED
)

if (MSVC)
	target_compile_definitions(Network_Lib
		PUBLIC
			_CRT_SECURE_NO_WARNINGS

			# ASIO related.
			_WIN32_WINNT=0x0A00 # _WIN32_WINNT_WIN10
			_WINSOCK_DEPRECATED_NO_WARNINGS
	)
	target_compile_options(Network_Lib
		PUBLIC
			/W3 /WX /permissive-
			/MP
	)
else ()
	if (CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		target_compile_options(Network_Lib PUBLIC --coverage)
		target_link_options(Network_Lib PUBLIC --coverage)
	endif ()
	target_compile_options(Network_Lib
		PRIVATE
			-Wall -pedantic -Wcast-align -Woverloaded-virtual -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function
	)
endif ()

target_link_libraries(Network_Lib
	PUBLIC
		asio asio::asio
		fmt::fmt
		Signal::Signal
		MWetris::Protocol
		MWetris::TetrisEngine_Lib
)

set_target_properties(Network_Lib
	PROPERTIES
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)
