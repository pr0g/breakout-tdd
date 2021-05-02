cmake_minimum_required(VERSION 3.15)
project(tdd-breakout LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/onqtam/doctest.git
  GIT_TAG 4d8716f1efc1d14aa736ef52ee727bd4204f4c40)
FetchContent_MakeAvailable(doctest)

find_package(Curses)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE Curses)

add_executable(${PROJECT_NAME}-test)
target_sources(${PROJECT_NAME}-test PRIVATE test.cpp)
target_link_libraries(${PROJECT_NAME}-test PRIVATE doctest)
target_compile_features(${PROJECT_NAME}-test PRIVATE cxx_std_17)
