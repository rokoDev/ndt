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
    ${MAIN_INCLUDE_DIR}/ndt/nomoveable.h
    ${MAIN_INCLUDE_DIR}/ndt/nodefaultconstructible.h
	${MAIN_INCLUDE_DIR}/ndt/executor_select_base.h
	${MAIN_INCLUDE_DIR}/ndt/platform/nix/executor_select_impl.h
    ${MAIN_INCLUDE_DIR}/ndt/platform/nix/context_base.h
	${MAIN_INCLUDE_DIR}/ndt/platform/win/executor_select_impl.h
    ${MAIN_INCLUDE_DIR}/ndt/platform/win/context_base.h
    ${MAIN_INCLUDE_DIR}/ndt/platform/win/context_base_error.h
    ${MAIN_INCLUDE_DIR}/ndt/event_handler_select.h
    ${MAIN_INCLUDE_DIR}/ndt/executor_select.h
    ${MAIN_INCLUDE_DIR}/ndt/bin_rw.h
    ${MAIN_INCLUDE_DIR}/ndt/endian.h
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
    ${SOURCE_DIR}/platform/win/context_base_error.cpp
	)

if(WIN32)
      # Don't need to compile this files for Windows platforms
      set_source_files_properties(
      	${MAIN_INCLUDE_DIR}/ndt/platform/nix/executor_select_impl.h
        ${MAIN_INCLUDE_DIR}/ndt/platform/nix/context_base.h
       PROPERTIES
          HEADER_FILE_ONLY YES
      )
else()
	# Don't need to compile this file for non-Windows platforms
      set_source_files_properties(
      	${MAIN_INCLUDE_DIR}/ndt/platform/win/executor_select_impl.h
        ${MAIN_INCLUDE_DIR}/ndt/platform/win/context_base.h
        ${MAIN_INCLUDE_DIR}/ndt/platform/win/context_base_error.h
        ${SOURCE_DIR}/platform/win/context_base_error.cpp
      	PROPERTIES
          HEADER_FILE_ONLY YES
      )
endif()

source_group(
	TREE   ${MAIN_INCLUDE_DIR}
	PREFIX "include"
	FILES  ${HEADERS}
)

source_group(
	TREE   ${SOURCE_DIR}
	PREFIX "src"
	FILES  ${SOURCES}
)

include_directories("${INCLUDE_DIRS}")

add_subdirectory(deps)

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
if(TEST_NDT OR CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # Setup testing
  add_subdirectory(tests)
endif()