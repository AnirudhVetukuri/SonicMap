cmake_minimum_required(VERSION 3.10)

project(SonicMap)

include_directories(${PROJECT_SOURCE_DIR}/include)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_Declare(
  xxhash
  GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git
  GIT_TAG v0.8.2
)
FetchContent_MakeAvailable(xxhash)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  tests_map
  tests/tests_map.cpp
)

target_link_libraries(
  tests_map
  GTest::gtest_main
  xxhash
)

include(GoogleTest)
gtest_discover_tests(tests_map)