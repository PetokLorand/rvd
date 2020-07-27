cmake_minimum_required(VERSION 3.1)

project(RequiredVacationCalculator)

add_executable(Release Main.cpp)
target_compile_options(Release PRIVATE -std=c++20 -Wall -Wextra -Wpedantic -Werror -O3)