cmake_minimum_required(VERSION 3.22)

project(RTP-Server CXX)

add_compile_options(-pie -Wall -pthread -std=c++20)

add_executable(${CMAKE_PROJECT_NAME})

set(SOURCE_DIR src)

set(SOURCE_FILES
	main.cpp)

foreach(path ${SOURCE_FILES})
	target_sources(${CMAKE_PROJECT_NAME} PRIVATE "${SOURCE_DIR}/${path}")
endforeach()
