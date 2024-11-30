project(Protocol)

find_package(Protobuf CONFIG REQUIRED)

set (PROTO_UTIL
	tp/util.cpp
	tp/util.h
)

set(PROTO_FILES
	"client_to_server.proto"
	"high_score.proto"
	"server_to_client.proto"
	"shared.proto"
)

make_directory(${CMAKE_CURRENT_BINARY_DIR}/src/protocol/) # Is needed for unix systems
protobuf_generate_cpp(
	PROTO_SRC PROTO_HDRS ${PROTO_FILES}
	PROTOC_OUT_DIR src/protocol/
)
add_library(Protocol ${PROTO_UTIL} ${PROTO_HDRS} ${PROTO_SRC} ${PROTO_FILES})
add_library(MWetris::Protocol ALIAS Protocol)

if (MSVC)
	target_compile_options(Protocol
		PUBLIC
			"/wd4005" # 'identifier' : macro redefinition
			"/wd4251" # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
			"/wd4244" # An integer type is converted to a smaller integer type
	)
endif ()

target_include_directories(Protocol
	PUBLIC
		${CMAKE_CURRENT_BINARY_DIR}/src
)

target_link_libraries(Protocol
	PUBLIC
		protobuf::libprotobuf
		protobuf::libprotobuf-lite
	PRIVATE
		protobuf::libprotoc
)

if (MSVC)
	target_compile_definitions(Protocol
		PRIVATE
			_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
	)
endif ()

set_target_properties(Protocol
	PROPERTIES
		CXX_STANDARD 23
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)
