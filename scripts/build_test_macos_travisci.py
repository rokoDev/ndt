#!/usr/bin/env python3

import os
import subprocess
import argparse
import shutil
import sys

def run_cmake(*args):
  command = ['cmake'] + list(args)
  try:
    subprocess.run(command, check=True)
  except subprocess.CalledProcessError as e:
    s = " "
    print(f"Command [{s.join(command)}] exit with code {e.returncode}")
    sys.exit(e.returncode)

def run_ctest(*args):
  command = ['ctest'] + list(args)
  try:
    subprocess.run(command, check=True)
  except subprocess.CalledProcessError as e:
    s = " "
    print(f"Command [{s.join(command)}] exit with code {e.returncode}")
    sys.exit(e.returncode)


def main():
  parser = argparse.ArgumentParser(description="""
  This script does:
    1. Create build directory and cd to it.
    2. Run CMake with specified generator type to generate project.
    3. Run CMake to build project.
    4. Run CTest to execute unit tests.
  """)
  parser.add_argument('--is_shared_libs', '-shared', action='store_true', help='If specified - build shared library and static otherwise.')
  parser.add_argument('--not_clear_build_dir', '-notclear', action='store_true', help='If specified - BUILD_DIR will not be cleared before cmake call')
  parser.add_argument('--build_type', default='Debug', choices=['Debug', 'Release'], help='Build type.')
  parser.add_argument('-g', default='Ninja', choices=['Xcode', 'Ninja'], help='CMake generator type.')
  parser.add_argument('-build_dir', default='./build', help='Relative or full path to build directory.')
  parser.add_argument('-source_dir', default='./', help='Relative or full path to directory with root CMakeLists.txt file.')
  
  args = parser.parse_args()

  IS_SHARED_LIBS = args.is_shared_libs
  NOT_CLEAR_BUILD_DIR = args.not_clear_build_dir
  BUILD_TYPE = args.build_type
  CMAKE_GENERATOR = args.g
  BUILD_DIR = os.path.abspath(args.build_dir)
  SOURCE_DIR = os.path.abspath(args.source_dir)

  print(f"IS_SHARED_LIBS:[{IS_SHARED_LIBS}]")
  print(f"NOT_CLEAR_BUILD_DIR:[{NOT_CLEAR_BUILD_DIR}]")
  print(f"BUILD_TYPE:[{BUILD_TYPE}]")
  print(f"CMAKE_GENERATOR:[{CMAKE_GENERATOR}]")
  print(f"BUILD_DIR:[{BUILD_DIR}]")
  print(f"SOURCE_DIR:[{SOURCE_DIR}]")

  if os.path.exists(BUILD_DIR):
    if not NOT_CLEAR_BUILD_DIR:
      shutil.rmtree(BUILD_DIR)
      os.makedirs(BUILD_DIR, exist_ok=True)
  else:
    os.makedirs(BUILD_DIR, exist_ok=True)

  print(f"\nCurrent working directory1:[{os.getcwd()}]\n")
  os.chdir(BUILD_DIR)
  print(f"\nCurrent working directory2:[{os.getcwd()}]\n")

  IS_SHARED_LIBS_STR = str(IS_SHARED_LIBS)

  print(f"CMake configure:[cmake -DBUILD_SHARED_LIBS={IS_SHARED_LIBS_STR} -DCMAKE_BUILD_TYPE={BUILD_TYPE} -G {CMAKE_GENERATOR} {SOURCE_DIR}]\n")
  run_cmake(f"-DBUILD_SHARED_LIBS={IS_SHARED_LIBS_STR}", f"-DCMAKE_BUILD_TYPE={BUILD_TYPE}", "-G", CMAKE_GENERATOR, SOURCE_DIR)

  print(f"\nCMake build:[cmake --build . --config {BUILD_TYPE}]\n")
  run_cmake('--build', '.', '--config', BUILD_TYPE)

  print(f"\nCTest run unit tests:[ctest -VV --output-on-failure -C {BUILD_TYPE}]")
  run_ctest('-VV', '--output-on-failure', '-C', BUILD_TYPE)

if __name__ == "__main__":
  main()
