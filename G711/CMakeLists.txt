cmake_minimum_required(VERSION 3.0)
project(G711)
include_directories(./g711)
aux_source_directory(./g711 SRC)

add_compile_options(-g  -fpermissive -std=c++14) 
set(CMAKE_BUILD_TYPE Release)
set(EXECUTABLE_OUTPUT_PATH ./)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./)

add_executable(example ./example.c ${SRC})

