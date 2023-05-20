include(FetchContent)
# Load external github projects
FetchContent_Declare(CppSdl2
	GIT_REPOSITORY
		https://github.com/mwthinker/CppSdl2.git
	GIT_TAG
		4435ce858c47ce765a3d43fae398aebde1ad3743
	OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(CppProtoNetwork
	GIT_REPOSITORY
		https://github.com/mwthinker/CppProtoNetwork.git
	GIT_TAG
		13dfa51590b971c67fc23e58615af6ea39125109
	OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(Signal
	GIT_REPOSITORY
		https://github.com/mwthinker/Signal.git
	GIT_TAG
		a4641c6d6880a704733fb0b90f3db56e48c543fe
	OVERRIDE_FIND_PACKAGE
)

fetchcontent_declare(Calculator
	GIT_REPOSITORY
		https://github.com/mwthinker/Calculator.git
	GIT_TAG
		7e552e5ce4338aa0e9399b54e5672c5d13b0f589
	OVERRIDE_FIND_PACKAGE
)

fetchcontent_declare(CmakeAuxiliary
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
