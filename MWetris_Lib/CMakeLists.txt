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
	src/graphic/imguiboard.cpp
	src/graphic/imguiboard.h
	src/graphic/gamecomponent.cpp
	src/graphic/gamecomponent.h
	
	src/game/actionhandler.cpp
	src/game/actionhandler.h
	src/game/computer.cpp
	src/game/computer.h
	src/game/creategame.cpp
	src/game/creategame.h
	src/game/devicemanager.cpp
	src/game/devicemanager.h
	src/game/device.h
	src/game/input.h
	src/game/gamepad.cpp
	src/game/gamepad.h
	src/game/keyboard.cpp
	src/game/keyboard.h
	src/game/localgame.cpp
	src/game/localgame.h
	src/game/localplayer.cpp
	src/game/localplayer.h
	src/game/localplayerbuilder.cpp
	src/game/localplayerbuilder.h
	src/game/player.cpp
	src/game/player.h
	src/game/sdldevice.h
	src/game/tetrisgame.cpp
	src/game/tetrisgameevent.h
	src/game/tetrisgame.h
	src/game/taskqueue.h
	src/game/serialize.cpp
	src/game/serialize.h
	
	src/ui/scene/customgame.cpp
	src/ui/scene/customgame.h
	src/ui/scene/highscore.cpp
	src/ui/scene/highscore.h
	src/ui/scene/settings.cpp
	src/ui/scene/settings.h
	src/ui/scene/network.cpp
	src/ui/scene/network.h
	src/ui/scene/newhighscore.cpp
	src/ui/scene/newhighscore.h
	src/ui/scene/scene.h
	src/ui/scene/statemachine.cpp
	src/ui/scene/statemachine.h

	src/ui/scene/about.cpp
	src/ui/scene/about.h

	src/ui/imguiextra.cpp
	src/ui/imguiextra.h
	src/ui/tetriswindow.cpp
	src/ui/tetriswindow.h
	
	src/configuration.cpp
	src/configuration.h
	src/types.h
)

find_package(Threads REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(CppProtoNetwork REQUIRED)
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
		src
)

target_compile_definitions(MWetris_Lib
	PUBLIC
		GIT_VERSION="${GIT_HASH}"
		PROJECT_VERSION="${CMAKE_PROJECT_VERSION}"
		GUID="${GUID}"
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
else ()
	target_compile_options(MWetris_Lib
		PRIVATE
			-Wall -pedantic -Wcast-align -Woverloaded-virtual -Wno-unused-parameter -Wno-sign-compare -Wno-unused-function
	)
endif ()

set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)

target_link_libraries(MWetris_Lib
	PUBLIC
		TetrisEngineLib
		nlohmann_json
		nlohmann_json::nlohmann_json
		Protocol
		Signal
		${ExternalDependencies}
)

set_target_properties(MWetris_Lib
	PROPERTIES
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)