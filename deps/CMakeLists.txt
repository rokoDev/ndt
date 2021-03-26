include(FetchContent)

FetchContent_Declare(fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG 9cb347b4b2e80fc9fbf57b8621746663c3f870f6)

FetchContent_MakeAvailable(fmt)