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
		${PROTOBUF_INCLUDE_DIR}
		${CMAKE_CURRENT_BINARY_DIR}
)

if (MSVC)
	target_compile_definitions(Protocol
		PRIVATE
			_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
	)
endif (MSVC)

set_target_properties(Protocol
	PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)
