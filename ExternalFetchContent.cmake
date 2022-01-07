include(FetchContent)
# Load external github projects
FetchContent_Declare(CppSdl2
	GIT_SHALLOW 1
	GIT_REPOSITORY
		https://github.com/mwthinker/CppSdl2.git
	GIT_TAG
		33433ef219817170ddcd573de99b4edd1ffe26bf
)
FetchContent_MakeAvailable(CppSdl2)

FetchContent_Declare(CppProtoNetwork
	GIT_SHALLOW 1
	GIT_REPOSITORY
		https://github.com/mwthinker/CppProtoNetwork.git
	GIT_TAG
		ad924a5c393c44b6cb54a86fe1a22758f870ea3a
)
FetchContent_MakeAvailable(CppProtoNetwork)

FetchContent_Declare(Signal
	GIT_SHALLOW 1
	GIT_REPOSITORY
		https://github.com/mwthinker/Signal.git
	GIT_TAG
		56a20197ab61a936cbdaace49c98fca50e1584ad
)
FetchContent_MakeAvailable(Signal)

fetchcontent_declare(CmakeAuxiliary
	GIT_SHALLOW 1
	GIT_REPOSITORY
		https://gist.github.com/mwthinker/e0539b200e3f0ac6da545af4843a6ff6
	GIT_TAG
		0f0aaa9201a5a4c11749cd5adfc496a3960014de
)
FetchContent_MakeAvailable(CmakeAuxiliary)
FetchContent_GetProperties(CmakeAuxiliary
	SOURCE_DIR
		CmakeAuxiliary_SOURCE_DIR
)
include(${CmakeAuxiliary_SOURCE_DIR}/auxiliary.cmake)

# Load data.
FetchContent_Declare(MWetrisData
	GIT_SHALLOW 1
	GIT_REPOSITORY
		https://github.com/mwthinker/MWetrisData.git
	GIT_TAG
		bb892f628dcb0cde99ca30f7b57e3f1b82e5fd2c
)
FetchContent_MakeAvailable(MWetrisData)
FetchContent_GetProperties(MWetrisData
	SOURCE_DIR
		MWetrisData_SOURCE_DIR
)
