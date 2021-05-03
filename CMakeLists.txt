set(cmake_version 3.18.3)

cmake_minimum_required(VERSION ${cmake_version})

set(ProjectName "ndt")

project(${ProjectName}
		VERSION 0.1.1
		LANGUAGES CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(TestBigEndian)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/VersionHelper")
include(VersionInfoUtils)
m_generate_version_info(PROJECT_NAME ${ProjectName}
	                    CPP_NAMESPACE "ndt"
	                    BUILD_TYPES Debug Release)

message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER_FRONTEND_VARIANT: ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")

list(APPEND UNIX_BUILD_TYPES_CLANG_FLAGS
	"-std=c++17"
	"-Werror"
	"-pedantic-errors"
	"-Wall"
	"-Wextra"
	"-Wconversion"
	"-Wsign-conversion"
	"-Wshadow-all"
	"-Wunreachable-code"
	"-Wuninitialized"
	"-Wold-style-cast"
	"-Wfloat-equal"
	"-Wcast-align"
	"-Winvalid-constexpr"
	)

list(APPEND WIN_BUILD_TYPES_CLANG_FLAGS
	"/std:c++17"
	"/WX"
	"/permissive-"
	"/W4"
	)

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL MSVC)
    list(APPEND ALL_CXX_FLAGS "${WIN_BUILD_TYPES_CLANG_FLAGS}")
  elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL GNU)
    list(APPEND ALL_CXX_FLAGS "${UNIX_BUILD_TYPES_CLANG_FLAGS}")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
  list(APPEND ALL_CXX_FLAGS "${UNIX_BUILD_TYPES_CLANG_FLAGS}")# "${CMAKE_CXX_FLAGS}"
endif()

message(STATUS "ALL_CXX_FLAGS: ${ALL_CXX_FLAGS}")

# Add dependencies
add_subdirectory(deps)

# Create library
add_library(${ProjectName} "")

# Add sources, definitions, link dependencies, etc.
add_subdirectory(sources)

# Create groups in the IDE which mirrors directory structure on the hard disk
get_target_property(ndt_sources ndt SOURCES)
source_group(
  TREE   ${CMAKE_CURRENT_SOURCE_DIR}/sources
  FILES  ${ndt_sources}
)

# Things typically only needed if we are the top level project
if(TEST_NDT OR CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Setup testing
  add_subdirectory(tests)
endif()