project(Protocol)

find_package(protobuf CONFIG REQUIRED)

set(PROTO_FILES
	"message.proto"
)

protobuf_generate_cpp(PROTO_SRC PROTO_HDRS ${PROTO_FILES})
add_library(Protocol ${PROTO_HDRS} ${PROTO_SRC} ${PROTO_FILES})
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
		${CMAKE_CURRENT_BINARY_DIR}
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
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)
