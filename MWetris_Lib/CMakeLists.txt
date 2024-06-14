project(MWetris_Lib
	DESCRIPTION
		"A lib for MWetris"
	LANGUAGES
		CXX
)

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4275") # non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251") # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
endif ()

set(SOURCES_LIB
	src/mwetris/graphic/gamecomponent.cpp
	src/mwetris/graphic/gamecomponent.h
	src/mwetris/graphic/imguiboard.cpp
	src/mwetris/graphic/imguiboard.h
	
	src/mwetris/game/actionhandler.cpp
	src/mwetris/game/actionhandler.h
	src/mwetris/game/computer.cpp
	src/mwetris/game/computer.h
	src/mwetris/game/defaultgamerules.h
	src/mwetris/game/devicemanager.cpp
	src/mwetris/game/devicemanager.h
	src/mwetris/game/device.h
	src/mwetris/game/input.h
	src/mwetris/game/gamepad.cpp
	src/mwetris/game/gamepad.h
	src/mwetris/game/gamerules.h
	src/mwetris/game/keyboard.cpp
	src/mwetris/game/keyboard.h
	src/mwetris/game/player.h
	src/mwetris/game/player.cpp
	src/mwetris/game/playerboardevent.h
	src/mwetris/game/playerslot.h
	src/mwetris/game/tetrisgame.cpp
	src/mwetris/game/tetrisgameevent.h
	src/mwetris/game/tetrisgame.h
	src/mwetris/game/tetrisboardcontroller.h
	src/mwetris/game/serialize.cpp
	src/mwetris/game/serialize.h
	src/mwetris/game/serializehighscore.cpp
	src/mwetris/game/serializehighscore.h

	src/mwetris/network/asio.h
	src/mwetris/network/client.h
	src/mwetris/network/debugclient.cpp
	src/mwetris/network/debugclient.h
	src/mwetris/network/debugserver.cpp
	src/mwetris/network/debugserver.h
	src/mwetris/network/gameroom.cpp
	src/mwetris/network/gameroom.h
	src/mwetris/network/id.cpp
	src/mwetris/network/id.h
	src/mwetris/network/network.cpp
	src/mwetris/network/network.h
	src/mwetris/network/networkevent.h
	src/mwetris/network/protobufmessage.cpp
	src/mwetris/network/protobufmessage.h
	src/mwetris/network/protobufmessagequeue.cpp
	src/mwetris/network/protobufmessagequeue.h
	src/mwetris/network/server.h
	src/mwetris/network/servercore.cpp
	src/mwetris/network/servercore.h
	src/mwetris/network/tcpclient.cpp
	src/mwetris/network/tcpclient.h
	src/mwetris/network/tcpserver.cpp
	src/mwetris/network/tcpserver.h
	
	src/mwetris/ui/scene/about.cpp
	src/mwetris/ui/scene/about.h
	src/mwetris/ui/scene/addplayer.cpp
	src/mwetris/ui/scene/addplayer.h
	src/mwetris/ui/scene/creategame.cpp
	src/mwetris/ui/scene/creategame.h
	src/mwetris/ui/scene/highscore.cpp
	src/mwetris/ui/scene/highscore.h
	src/mwetris/ui/scene/joingame.cpp
	src/mwetris/ui/scene/joingame.h
	src/mwetris/ui/scene/newhighscore.cpp
	src/mwetris/ui/scene/newhighscore.h
	src/mwetris/ui/scene/scene.h
	src/mwetris/ui/scene/settings.cpp
	src/mwetris/ui/scene/settings.h
	src/mwetris/ui/scene/statemachine.cpp
	src/mwetris/ui/scene/statemachine.h

	src/mwetris/ui/imguiextra.cpp
	src/mwetris/ui/imguiextra.h
	src/mwetris/ui/networkdebugwindow.cpp
	src/mwetris/ui/networkdebugwindow.h
	src/mwetris/ui/subwindow.h
	src/mwetris/ui/tetriswindow.cpp
	src/mwetris/ui/tetriswindow.h

	src/mwetris/util/protofile.h
	src/mwetris/util/uuid.cpp
	src/mwetris/util/uuid.h
	
	src/mwetris/tetriscontroller.cpp
	src/mwetris/tetriscontroller.h
	src/mwetris/configuration.cpp
	src/mwetris/configuration.h
	src/mwetris/timerhandler.cpp
	src/mwetris/timerhandler.h
	src/mwetris/types.h
	src/mwetris/util.h
)

find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Signal REQUIRED)
find_package(CppSdl2 REQUIRED)

set_property(GLOBAL PROPERTY USE_FOLDERS On)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MWetris_Lib)
source_group(TREE
	${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${SOURCES_LIB}
)

add_library(MWetris_Lib ${SOURCES_LIB})

target_include_directories(MWetris_Lib
	PUBLIC
		src/
	PRIVATE
		src/mwetris/
)

target_compile_definitions(MWetris_Lib
	PUBLIC
		GIT_VERSION="${GIT_HASH}"
		PROJECT_VERSION="${CMAKE_PROJECT_VERSION}"
		MWETRIS_GUID="${GUID}"
)

# ASIO non-boost version.
target_compile_definitions(MWetris_Lib
	PUBLIC
		ASIO_STANDALONE
		ASIO_NO_DEPRECATED
)
target_compile_definitions(MWetris_Lib
	PUBLIC
		ASIO_NO_DEPRECATED
		ASIO_NO_DYNAMIC_BUFFER_V1
)

if (MSVC)
	target_compile_definitions(MWetris_Lib
		PUBLIC
			_CRT_SECURE_NO_WARNINGS
			NOMINMAX
	)
	target_compile_options(MWetris_Lib
		PUBLIC
			/W3 /WX /permissive-
			/MP
	)

	# ASIO related.
	target_compile_definitions(MWetris_Lib
		PUBLIC
			_WIN32_WINNT=_WIN32_WINNT_WS03
			_WINSOCK_DEPRECATED_NO_WARNINGS
	)
else ()
	if (CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		target_compile_options(MWetris_Lib PUBLIC --coverage)
		target_link_options(MWetris_Lib PUBLIC --coverage)
	endif ()
	target_compile_options(MWetris_Lib
		PRIVATE
			-Wall -pedantic -Wcast-align -Woverloaded-virtual -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function
	)
endif ()

set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)

target_link_libraries(MWetris_Lib
	PUBLIC
		asio asio::asio
		TetrisEngineLib
		nlohmann_json
		nlohmann_json::nlohmann_json
		fmt::fmt
		Protocol
		Signal
		${ExternalDependencies}
)

set_target_properties(MWetris_Lib
	PROPERTIES
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)
