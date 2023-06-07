cmake_minimum_required(VERSION 3.1)

project(powmon VERSION 1.0
                  DESCRIPTION "csv-like storage to sqlite"
                  LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(demo demo.cc sqlwriter.cc)
target_link_libraries(demo  sqlite3  pthread  dl  )

add_executable(pqdemo pqdemo.cc psqlwriter.cc minipsql.cc)
target_link_libraries(pqdemo pthread  dl pq )


add_executable(testrunner testrunner.cc sqlwriter.cc )
target_link_libraries(testrunner sqlite3 pthread dl)

enable_testing()
add_test(testname testrunner)
