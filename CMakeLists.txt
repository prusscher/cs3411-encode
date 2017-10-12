cmake_minimum_required(VERSION 3.8)
project(prog2)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        encode.c
        makefile
        util.c
        util.h
        strings.h)

add_executable(prog2 ${SOURCE_FILES})