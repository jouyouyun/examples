cmake_minimum_required(VERSION 3.1)

PROJECT(hostname-manager)

if (WIN32)
find_package(Catch2 REQUIRED)
#find_package(googletest REQUIRED)
endif(WIN32)

add_executable(catch_test catch_test.cpp hostname.cpp)
# needed compile gtest
add_executable(google_test google_test.cpp hostname.cpp)
target_link_libraries(google_test gtestd)

enable_testing()
