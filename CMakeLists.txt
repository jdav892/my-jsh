cmake_minimum_required(VERSION 3.10)
project(jsh C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

#Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

#Source files
set(SOURCES
  main.c
  builtin.c
  parser.c
  history.c
  pipe.c
  utils.c
  execute.c
  jsh.h
)

#Create executable
add_executable(jsh ${SOURCES})

#Installation
install(TARGETS jsh DESTINATION bin)

#Optional: Add custom target for cleaing
add_custom_target(clean-all
  COMMAND ${CMAKE_BUILD_TOOL} clean
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/cmake_install.cmake
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/Makefile
  COMMENT "Cleaning all build files"
)
