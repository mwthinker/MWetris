cmake_minimum_required(VERSION 3.0)
project(Protocol)

find_package(Protobuf REQUIRED)

set(PROTO_FILES
	"message.proto"
)

# Hide options in cmake
mark_as_advanced(FORCE Protobuf_SRC_ROOT_FOLDER)
mark_as_advanced(FORCE Protobuf_DIR)

PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER ${PROTO_FILES})
add_library(Protocol ${PROTO_HEADER} ${PROTO_SRC} ${PROTO_FILES})

target_include_directories(Protocol
	PUBLIC
		${PROTOBUF_INCLUDE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}
)

if (MSVC)
	target_compile_definitions(Protocol
		PRIVATE
			_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
	)
endif (MSVC)
