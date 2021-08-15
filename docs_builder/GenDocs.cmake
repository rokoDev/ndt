find_package(Git REQUIRED)

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

# OUTPUT_DIRECTORY will be substituted after call to configure_file
set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)

# Generate Doxyfile
configure_file(${DOXYGEN_IN} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

# Delete directory with documentation generated prevously
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/docs")

# Generate documentation
execute_process(
	COMMAND ${PATH_TO_DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	RESULT_VARIABLE result
)

if(result)
	message(FATAL_ERROR "Failed to generate docs: ${result}")
endif()