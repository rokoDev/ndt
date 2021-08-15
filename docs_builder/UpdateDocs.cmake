file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/gh-pages")

find_package(Git REQUIRED)

message(STATUS "Extracting git origin url...")
execute_process(
    COMMAND ${GIT_EXECUTABLE} config --get remote.origin.url
    RESULT_VARIABLE result
    OUTPUT_VARIABLE GIT_ORIGIN_URL
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(result)
	message(FATAL_ERROR "Failed to get git origin url: ${result}")
else()
	message(STATUS "GIT_ORIGIN_URL:${GIT_ORIGIN_URL}")
endif()

message(STATUS "Cloning the gh-pages branch into the gh-pages directory...")
execute_process(
    COMMAND ${GIT_EXECUTABLE} clone -b gh-pages --single-branch ${GIT_ORIGIN_URL} --depth 1 "${CMAKE_CURRENT_BINARY_DIR}/gh-pages"
    RESULT_VARIABLE result
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(result)
    message(FATAL_ERROR "failed to clone gh-pages branch: ${result}")
endif()

message(STATUS "Cleaning up ${CMAKE_CURRENT_BINARY_DIR}/gh-pages/docs/...")
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/gh-pages/docs")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/gh-pages/docs")

message(STATUS "Copying the documentation from ${CMAKE_CURRENT_BINARY_DIR}/docs/html to ${CMAKE_CURRENT_BINARY_DIR}/gh-pages/docs...")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/docs/html" "${CMAKE_CURRENT_BINARY_DIR}/gh-pages/docs"
    RESULT_VARIABLE result
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(result)
	message(FATAL_ERROR "Failed to copy the documentation: ${result}")
endif()

# Getting last commit hash
execute_process(
	COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
	RESULT_VARIABLE result
	OUTPUT_VARIABLE GIT_SHA1
	OUTPUT_STRIP_TRAILING_WHITESPACE
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

if(result)
	message(FATAL_ERROR "Failed to get hash of last change: ${result}")
endif()

# Getting diff since last commit(e.g. any uncommited changes)
execute_process(
	COMMAND ${GIT_EXECUTABLE} diff HEAD
	RESULT_VARIABLE result
	OUTPUT_VARIABLE CURRENT_DIFF
	OUTPUT_STRIP_TRAILING_WHITESPACE
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

if(result)
	message(FATAL_ERROR "Failed to get diff since last commit: ${result}")
endif()

# If there are no uncommited changes set IS_DIRTY flag to "false"
set(IS_DIRTY true)
if("${CURRENT_DIFF}" STREQUAL "")
	set(IS_DIRTY false)
endif()

message(STATUS "Adding changes in the documentation to git index...")
execute_process(
    COMMAND ${GIT_EXECUTABLE} add --all .
    RESULT_VARIABLE result
	OUTPUT_STRIP_TRAILING_WHITESPACE
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gh-pages
)

if(result)
	message(FATAL_ERROR "Failed to add changes to git index: ${result}")
endif()

message(STATUS "Making git commit...")
execute_process(
    COMMAND ${GIT_EXECUTABLE} commit --allow-empty -m "Docs for ${GIT_SHA1}(IS_DIRTY: ${IS_DIRTY})"
    RESULT_VARIABLE result
	OUTPUT_STRIP_TRAILING_WHITESPACE
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gh-pages
)

if(result)
	message(FATAL_ERROR "Failed to commit the gh-pages branch: ${result}")
endif()