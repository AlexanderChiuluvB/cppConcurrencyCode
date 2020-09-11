cmake_minimum_required(VERSION 3.16)
project(threadPool)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -lpthread")
add_executable(cpp17 threadPool.cpp)