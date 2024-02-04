cmake_minimum_required(VERSION 3.27)
project(qgi)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(qgi
        src/main.cpp
        src/qoi.cpp
        src/qoi.h
        src/qgi.h
        src/qgi.cpp
        src/benchmark.cpp
        src/benchmark.h
        src/stb_image.h)
