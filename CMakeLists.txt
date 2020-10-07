cmake_minimum_required(VERSION 3.0)
project(sudoku-sat)

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(kissat GIT_REPOSITORY "https://github.com/lakshayg/kissat")
FetchContent_GetProperties(kissat)
if(NOT kissat_POPULATED)
  FetchContent_Populate(kissat)
  add_subdirectory(${kissat_SOURCE_DIR} ${kissat_BINARY_DIR})
  set(KISSAT_INCLUDE_DIR ${kissat_SOURCE_DIR}/src)
endif()

FetchContent_Declare(minisat GIT_REPOSITORY "https://github.com/lakshayg/minisat")
FetchContent_GetProperties(minisat)
if(NOT minisat_POPULATED)
  FetchContent_Populate(minisat)
  add_subdirectory(${minisat_SOURCE_DIR} ${minisat_BINARY_DIR})
  set(MINISAT_INCLUDE_DIR ${minisat_SOURCE_DIR})
endif()

set(CMAKE_C_FLAGS "-Wall -pedantic -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} --std=c++11")

add_executable(kissat_main kissat_main.c)
target_include_directories(kissat_main PRIVATE ${KISSAT_INCLUDE_DIR})
target_link_libraries(kissat_main kissat)

add_executable(minisat_main minisat_main.cpp)
target_include_directories(minisat_main PRIVATE ${MINISAT_INCLUDE_DIR})
target_link_libraries(minisat_main minisat-lib-static)
