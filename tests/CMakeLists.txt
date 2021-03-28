cmake_minimum_required(VERSION ${cmake_version})

set(outProjectName ${ProjectName})
set(ProjectName ${ProjectName}_tests)
project(${ProjectName})

FetchContent_Declare(googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG df7fee587d442b372ef43bd66c6a2f5c9af8c5eb)

FetchContent_MakeAvailable(googletest)

if (WIN32)
	option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)
	option(GTEST_DISABLE_PTHREADS "Disable uses of pthreads in gtest." ON)
    option(GTEST_FORCE_SHARED_CRT "Use shared (DLL) run-time lib even when Google Test is built as static lib." ON)
endif (WIN32)

macro(package_add_test TESTNAME)
	include_directories("include")
    add_executable(${TESTNAME} ${ARGN})
    target_link_libraries(${TESTNAME} gtest gmock gtest_main ${outProjectName})
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

package_add_test(${ProjectName}
	src/main_tests.cpp
	src/utils_tests.cpp
	src/address_tests.cpp
	src/socket_tests.cpp
	src/thread_pool_tests.cpp
	src/context_tests.cpp
	)