cmake_minimum_required(VERSION 3.20)

project(wochat)

add_definitions(-D_UNICODE -DUNICODE -DNOMINMAX)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(win)
add_subdirectory(wt)
add_subdirectory(dui)
add_subdirectory(mosquitto)
add_subdirectory(secp256k1)
add_subdirectory(sqlite)
add_subdirectory(markdown)
add_subdirectory(bitcoin)
