set(cmake_version 3.18.3)

cmake_minimum_required(VERSION ${cmake_version})

set(ProjectName "ndt")

project(${ProjectName}
		VERSION 0.1.1
		LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
	${MAIN_INCLUDE_DIR}/ndt/common.h
	${MAIN_INCLUDE_DIR}/ndt/socket.h
	${MAIN_INCLUDE_DIR}/ndt/utils.h
	${MAIN_INCLUDE_DIR}/ndt/fast_pimpl.h
	${MAIN_INCLUDE_DIR}/ndt/udp.h
	${MAIN_INCLUDE_DIR}/ndt/address.h
	${MAIN_INCLUDE_DIR}/ndt/exception.h
	${MAIN_INCLUDE_DIR}/ndt/thread_pool.h
	${MAIN_INCLUDE_DIR}/ndt/buffer.h
	${MAIN_INCLUDE_DIR}/ndt/system_wrappers.h
	${MAIN_INCLUDE_DIR}/ndt/context.h
	${MAIN_INCLUDE_DIR}/ndt/nocopyable.h
	)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SOURCES
	${SOURCE_DIR}/utils.cpp
	${SOURCE_DIR}/udp.cpp
	${SOURCE_DIR}/address.cpp
	${SOURCE_DIR}/exception.cpp
	${SOURCE_DIR}/common.cpp
	${SOURCE_DIR}/thread_pool.cpp
	${SOURCE_DIR}/system_wrappers.cpp
	${SOURCE_DIR}/context.cpp
	)

include_directories("${INCLUDE_DIRS}")

if(NOT TARGET fmt)
  add_subdirectory(fmt)
endif()

# Create library
message(STATUS "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
add_library(${ProjectName} ${SOURCES} ${HEADERS})
target_compile_definitions(${ProjectName} PUBLIC $<UPPER_CASE:$<CONFIG>>)
target_compile_options(${ProjectName} PRIVATE ${ALL_CXX_FLAGS})
set_target_properties(${ProjectName} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${ProjectName} PUBLIC "${ProjectName}_versionInfo" fmt)
target_include_directories(${ProjectName} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Things typically only needed if we are the top level project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Setup testing
  add_subdirectory(googletest)
  enable_testing()
  add_subdirectory(tests)
endif()