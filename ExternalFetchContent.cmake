include(FetchContent)
# Load external github projects
FetchContent_Declare(CppSdl2
	GIT_REPOSITORY
		https://github.com/mwthinker/CppSdl2.git
	GIT_TAG
		673645d9f03d0582229f6912178e7061bca663bf
)
FetchContent_MakeAvailable(CppSdl2)

FetchContent_Declare(CppProtoNetwork
	GIT_REPOSITORY
		https://github.com/mwthinker/CppProtoNetwork.git
	GIT_TAG
		b4045c7a20871ed7b00bf303ba472b717f9ecd5f
)
FetchContent_MakeAvailable(CppProtoNetwork)

FetchContent_Declare(Signal
	GIT_REPOSITORY
		https://github.com/mwthinker/Signal.git
	GIT_TAG
		2ce1a5505b167b76cf8ad7caaf4a0a527925cc08
)
FetchContent_MakeAvailable(Signal)

fetchcontent_declare(CmakeAuxiliary
	GIT_REPOSITORY
		https://gist.github.com/mwthinker/e0539b200e3f0ac6da545af4843a6ff6
)
FetchContent_MakeAvailable(CmakeAuxiliary)
FetchContent_GetProperties(CmakeAuxiliary
	SOURCE_DIR
		CmakeAuxiliary_SOURCE_DIR
)
include(${CmakeAuxiliary_SOURCE_DIR}/auxiliary.cmake)

# Load data.
FetchContent_Declare(MWetrisData
	GIT_REPOSITORY
		https://github.com/mwthinker/MWetrisData.git
	GIT_TAG
		d9bfd28c799b71e6048b51ae7f591f4853b7e887
)
FetchContent_MakeAvailable(MWetrisData)
FetchContent_GetProperties(MWetrisData
	SOURCE_DIR
		MWetrisData_SOURCE_DIR
)
