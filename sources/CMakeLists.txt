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

# Choose c++ compiler flags based on the compiler available or exit with error if no known compiler
if(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  if("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")
    list(APPEND ALL_CXX_FLAGS "${WIN_BUILD_TYPES_CLANG_FLAGS}")
  elseif("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "GNU")
    list(APPEND ALL_CXX_FLAGS "${UNIX_BUILD_TYPES_CLANG_FLAGS}")
  else()
    message(FATAL_ERROR "CMAKE_CXX_COMPILER_FRONTEND_VARIANT: ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}(Unrecognized)")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
  list(APPEND ALL_CXX_FLAGS "${UNIX_BUILD_TYPES_CLANG_FLAGS}")# "${CMAKE_CXX_FLAGS}"
else()
  message(FATAL_ERROR "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}(Unrecognized)")
endif()

message(STATUS "ALL_CXX_FLAGS: ${ALL_CXX_FLAGS}")

target_sources(ndt
	PRIVATE
    include/ndt/core.h
    include/ndt/common.h
    include/ndt/socket.h
    include/ndt/utils.h
    include/ndt/fast_pimpl.h
    include/ndt/udp.h
    include/ndt/address.h
    include/ndt/exception.h
    include/ndt/thread_pool.h
    include/ndt/buffer.h
    include/ndt/sys_socket_ops.h
    include/ndt/context.h
    include/ndt/useful_base_types.h
    include/ndt/executor_select_base.h
    include/ndt/platform/nix/executor_select_impl.h
    include/ndt/platform/nix/context_base.h
    include/ndt/platform/win/executor_select_impl.h
    include/ndt/platform/win/context_base.h
    include/ndt/platform/win/context_base_error.h
    include/ndt/event_handler_select.h
    include/ndt/executor_select.h
    include/ndt/bin_rw.h
    include/ndt/endian.h
    include/ndt/index_maker.h
    include/ndt/packet.h
    include/ndt/packet_handlers.h
    include/ndt/sys_file_ops.h
    include/ndt/sys_error_code.h
    include/ndt/file.h

    src/utils.cpp
    src/udp.cpp
    src/address.cpp
    src/exception.cpp
    src/common.cpp
    src/thread_pool.cpp
    src/sys_socket_ops.cpp
    src/platform/win/context_base_error.cpp
    src/endian.cpp
    src/packet.cpp
    src/sys_file_ops.cpp
    src/file.cpp
  )

set(MAIN_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

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

include(TestBigEndian)
test_big_endian(HOST_BIG_ENDIAN)
if(HOST_BIG_ENDIAN)
    target_compile_definitions(ndt PRIVATE HOST_BIG_ENDIAN)
endif()

target_compile_definitions(ndt PRIVATE $<UPPER_CASE:$<CONFIG>>)
target_compile_options(ndt PRIVATE ${ALL_CXX_FLAGS})
set_target_properties(ndt PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(ndt PUBLIC fmt)
target_include_directories(ndt PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)