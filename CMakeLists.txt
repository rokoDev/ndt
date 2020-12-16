set(cmake_version 3.18.3)

cmake_minimum_required(VERSION ${cmake_version})

set(ProjectName "ndt")

project(${ProjectName}
		VERSION 0.1.0
		LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/VersionHelper")
include(VersionInfoUtils)
m_generate_version_info_sources_by_project_name(MY_PROJECT_NAME ${ProjectName})

message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER_FRONTEND_VARIANT: ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
  if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /WX /permissive- /W4")
  elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Werror -pedantic-errors -Wall -Wextra -Wconversion -Wsign-conversion -Wshadow")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Werror -pedantic-errors -Wall -Wextra -Wconversion -Wsign-conversion -Wshadow")
endif()

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
set_target_properties(${ProjectName} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${ProjectName} PUBLIC versionInfo fmt)

# Setup testing
add_subdirectory(googletest)
enable_testing()
add_subdirectory(tests)