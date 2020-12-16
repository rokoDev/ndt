cmake_minimum_required(VERSION ${cmake_version})

set(outProjectName ${ProjectName})
set(ProjectName ${ProjectName}_tests)
project(${ProjectName})

if (WIN32)
	option(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)
	option(GTEST_DISABLE_PTHREADS "Disable uses of pthreads in gtest." ON)
    option(GTEST_FORCE_SHARED_CRT "Use shared (DLL) run-time lib even when Google Test is built as static lib." ON)
endif (WIN32)

macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_link_libraries(${TESTNAME} gtest gmock gtest_main ${outProjectName})
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

package_add_test(${ProjectName}
	main_tests.cpp
	Memvcmp_tests.cpp
	Address_tests.cpp
	Socket_tests.cpp
	)