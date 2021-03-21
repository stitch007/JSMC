cmake_minimum_required(VERSION 3.17)
project(JSMC C)

set(CMAKE_C_STANDARD 11)

add_executable(JSMC main.c jsmc.c)
