cmake_minimum_required(VERSION 3.1)

project(powmon VERSION 1.0
                  DESCRIPTION "Convert DSMR and GPIO pulses to Prometheus"
                  LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)


add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ext/SQLiteCpp)

add_executable(demo demo.cc sqlwriter.cc)
target_link_libraries(demo
  SQLiteCpp
  sqlite3
  pthread
  dl
  )


