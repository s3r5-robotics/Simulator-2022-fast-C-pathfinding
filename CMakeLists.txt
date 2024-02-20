cmake_minimum_required(VERSION 3.27)
project(C_implementation_of_pathFinding C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(C_implementation_of_pathFinding
        heap.h
        main.c
        pathfinding.h)
