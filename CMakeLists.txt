cmake_minimum_required(VERSION 3.20)

project(RoyC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER "C:/msys64/ucrt64/bin/g++")
set(CMAKE_C_COMPILER "C:/msys64/ucrt64/bin/g++")


add_executable(RoyC src/main.cpp
        src/tokenization.hpp
        src/parser.hpp
        src/generation.hpp
        src/arena.hpp)