include(FetchContent)

get_filename_component(deps_content "../deps_content"
                       REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(FETCHCONTENT_BASE_DIR ${deps_content})

FetchContent_Declare(fmt
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG 9cb347b4b2e80fc9fbf57b8621746663c3f870f6)

FetchContent_MakeAvailable(fmt)