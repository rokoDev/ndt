set(cmake_version 3.18.3)

cmake_minimum_required(VERSION ${cmake_version})

set(ProjectName "ndt")

set(CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY ON)

project(${ProjectName}
		VERSION 0.1.0
		LANGUAGES CXX)

set(allowableBuildTypes Debug Release)
get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(isMultiConfig)
	set(CMAKE_CONFIGURATION_TYPES "${allowableBuildTypes}")
else()
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowableBuildTypes}")
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
	elseif(NOT CMAKE_BUILD_TYPE IN_LIST allowableBuildTypes)
		message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
	endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/VersionHelper")
include(VersionInfoUtils)
m_generate_version_info_sources_by_project_name(MY_PROJECT_NAME ${ProjectName})

message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER_FRONTEND_VARIANT: ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND UNIX_BUILD_TYPES_CLANG_FLAGS
	"-std=c++17"
	"-Werror"
	"-pedantic-errors"
	"-Wall"
	"-Wextra"
	"-Wconversion"
	"-Wsign-conversion"
	"-Wshadow"
	"-Wunreachable-code"
	"-Wuninitialized"
	"-Wold-style-cast"
	"-Wfloat-equal"
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

set(MAIN_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

set(INCLUDE_DIRS
	${MAIN_INCLUDE_DIR}
	${CMAKE_CURRENT_LIST_DIR}/fmt/include
	)
set(HEADERS
	${MAIN_INCLUDE_DIR}/CommonInclude.h
	${MAIN_INCLUDE_DIR}/Socket.h
	${MAIN_INCLUDE_DIR}/NetUtils.h
	${MAIN_INCLUDE_DIR}/FastPimpl.h
	${MAIN_INCLUDE_DIR}/UDP.h
	${MAIN_INCLUDE_DIR}/Address.h
	${MAIN_INCLUDE_DIR}/NetException.h
	)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(SOURCES
	${SOURCE_DIR}/NetUtils.cpp
	${SOURCE_DIR}/UDP.cpp
	${SOURCE_DIR}/Address.cpp
	${SOURCE_DIR}/NetException.cpp
	${SOURCE_DIR}/Socket.cpp
	)

include_directories("${INCLUDE_DIRS}")

add_subdirectory(fmt)

# Create library
message(STATUS "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
add_library(${ProjectName} ${SOURCES} ${HEADERS})
target_compile_definitions(${ProjectName} PUBLIC $<$<CONFIG:Release>:RELEASE> $<$<CONFIG:Debug>:DEBUG>)
target_compile_options(${ProjectName} PRIVATE ${ALL_CXX_FLAGS})
set_target_properties(${ProjectName} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${ProjectName} PUBLIC versionInfo fmt)

# Setup testing
add_subdirectory(googletest)
enable_testing()
add_subdirectory(tests)