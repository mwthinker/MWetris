project(App_Lib
	DESCRIPTION
		"A lib for App"
	LANGUAGES
		CXX
)

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4275") # non - DLL-interface class 'class_1' used as base for DLL-interface class 'class_2'
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251") # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
endif ()

set(SOURCES_LIB
	src/app/graphic/gamecomponent.cpp
	src/app/graphic/gamecomponent.h
	src/app/graphic/imguiboard.cpp
	src/app/graphic/imguiboard.h
	
	src/app/game/actionhandler.cpp
	src/app/game/actionhandler.h
	src/app/game/computer.cpp
	src/app/game/computer.h
	src/app/game/defaultgamerules.h
	src/app/game/devicemanager.cpp
	src/app/game/devicemanager.h
	src/app/game/device.h
	src/app/game/input.h
	src/app/game/gamepad.cpp
	src/app/game/gamepad.h
	src/app/game/gamerules.h
	src/app/game/keyboard.cpp
	src/app/game/keyboard.h
	src/app/game/player.h
	src/app/game/player.cpp
	src/app/game/playerboardevent.h
	src/app/game/playerslot.h
	src/app/game/tetrisgame.cpp
	src/app/game/tetrisgameevent.h
	src/app/game/tetrisgame.h
	src/app/game/tetrisboardcontroller.h
	src/app/game/serialize.cpp
	src/app/game/serialize.h
	src/app/game/serializehighscore.cpp
	src/app/game/serializehighscore.h
	src/app/game/tetrisparameters.h

	src/app/cnetwork/network.cpp
	src/app/cnetwork/network.h
	src/app/cnetwork/networkevent.h
	
	src/app/ui/scene/about.cpp
	src/app/ui/scene/about.h
	src/app/ui/scene/addplayer.cpp
	src/app/ui/scene/addplayer.h
	src/app/ui/scene/creategameroom.cpp
	src/app/ui/scene/creategameroom.h
	src/app/ui/scene/gameroomlobby.cpp
	src/app/ui/scene/gameroomlobby.h
	src/app/ui/scene/highscore.cpp
	src/app/ui/scene/highscore.h
	src/app/ui/scene/joingameroom.cpp
	src/app/ui/scene/joingameroom.h
	src/app/ui/scene/networkerror.cpp
	src/app/ui/scene/networkerror.h
	src/app/ui/scene/newhighscore.cpp
	src/app/ui/scene/newhighscore.h
	src/app/ui/scene/play.cpp
	src/app/ui/scene/play.h
	src/app/ui/scene/resume.cpp
	src/app/ui/scene/resume.h
	src/app/ui/scene/scene.h
	src/app/ui/scene/settings.cpp
	src/app/ui/scene/settings.h
	src/app/ui/scene/statemachine.cpp
	src/app/ui/scene/statemachine.h

	src/app/ui/imguiextra.cpp
	src/app/ui/imguiextra.h
	src/app/ui/networkdebugwindow.cpp
	src/app/ui/networkdebugwindow.h
	src/app/ui/subwindow.h
	src/app/ui/tetriswindow.cpp
	src/app/ui/tetriswindow.h

	src/app/util/auxiliary.h
	src/app/util/protofile.h
	src/app/util/uuid.cpp
	src/app/util/uuid.h
	
	src/app/tetriscontroller.cpp
	src/app/tetriscontroller.h
	src/app/configuration.cpp
	src/app/configuration.h
	src/app/timerhandler.cpp
	src/app/timerhandler.h
	src/app/types.h
)

find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Signal REQUIRED)
find_package(CppSdl2 REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set_property(GLOBAL PROPERTY USE_FOLDERS On)
source_group(TREE
	${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${SOURCES_LIB}
)

add_library(App_Lib ${SOURCES_LIB})
add_library(MWetris::App_Lib ALIAS App_Lib)

target_include_directories(App_Lib
	PUBLIC
		src/
	PRIVATE
		src/app/
		${IconFontCppHeaders_SOURCE_DIR}
)


target_compile_definitions(App_Lib
	PUBLIC
		GIT_VERSION="${GIT_HASH}"
		GIT_DATE="${GIT_DATE}"
		VCPKG_HASH="${VCPKG_HASH}"
		VCPKG_DATE="${VCPKG_DATE}"
		PROJECT_VERSION="${CMAKE_PROJECT_VERSION}"
		MWETRIS_GUID="${GUID}"
)

if (MSVC)
	target_compile_definitions(App_Lib
		PUBLIC
			NOMINMAX
	)
	target_compile_options(App_Lib
		PUBLIC
			/W3 /WX /permissive-
			/MP
	)
else ()
	if (CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		target_compile_options(App_Lib PUBLIC --coverage)
		target_link_options(App_Lib PUBLIC --coverage)
	endif ()
	target_compile_options(App_Lib
		PRIVATE
			-Wall -pedantic -Wcast-align -Woverloaded-virtual -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function
	)
endif ()

set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)

target_link_libraries(App_Lib
	PUBLIC
		fmt::fmt
		nlohmann_json::nlohmann_json
		Signal::Signal
		CppSdl2::CppSdl2
		Calculator::Calculator
		MWetris::TetrisEngineLib
		MWetris::Network_Lib
		MWetris::Protocol
)

set_target_properties(App_Lib
	PROPERTIES
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)
