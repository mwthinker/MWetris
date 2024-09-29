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
	src/mwetris/game/tetrisparameters.h

	src/mwetris/cnetwork/network.cpp
	src/mwetris/cnetwork/network.h
	src/mwetris/cnetwork/networkevent.h
	
	src/mwetris/ui/scene/about.cpp
	src/mwetris/ui/scene/about.h
	src/mwetris/ui/scene/addplayer.cpp
	src/mwetris/ui/scene/addplayer.h
	src/mwetris/ui/scene/creategameroom.cpp
	src/mwetris/ui/scene/creategameroom.h
	src/mwetris/ui/scene/gameroomlobby.cpp
	src/mwetris/ui/scene/gameroomlobby.h
	src/mwetris/ui/scene/highscore.cpp
	src/mwetris/ui/scene/highscore.h
	src/mwetris/ui/scene/joingameroom.cpp
	src/mwetris/ui/scene/joingameroom.h
	src/mwetris/ui/scene/networkerror.cpp
	src/mwetris/ui/scene/networkerror.h
	src/mwetris/ui/scene/newhighscore.cpp
	src/mwetris/ui/scene/newhighscore.h
	src/mwetris/ui/scene/play.cpp
	src/mwetris/ui/scene/play.h
	src/mwetris/ui/scene/resume.cpp
	src/mwetris/ui/scene/resume.h
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

	src/mwetris/util/auxiliary.h
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

target_include_directories(App_Lib
	PUBLIC
		src/
	PRIVATE
		src/mwetris/
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
		TetrisEngineLib
		Network_Lib
		nlohmann_json
		nlohmann_json::nlohmann_json
		fmt::fmt
		Protocol
		Signal
		${ExternalDependencies}
)

set_target_properties(App_Lib
	PROPERTIES
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)
