cmake_minimum_required(VERSION 3.22)

project(RTP-Server CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-pie -Wall -pthread)

# Source Files
set(SOURCE_DIR src)

add_executable(${CMAKE_PROJECT_NAME})

set(SOURCE_FILES
	main.cpp
	Defer.cpp)

foreach(path ${SOURCE_FILES})
	target_sources(${CMAKE_PROJECT_NAME} PRIVATE "${SOURCE_DIR}/${path}")
endforeach()

# Libraries
set(SERVER_LIB Server)

add_library(${SERVER_LIB})

set(SERVER_FILES
	Server.cpp
	Connection.cpp)

foreach(path ${SERVER_FILES})
	target_sources(${SERVER_LIB} PRIVATE "${SOURCE_DIR}/${SERVER_LIB}/${path}")
endforeach()

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${SERVER_LIB})

# Tests
if (BUILD_TESTS)
	add_subdirectory(test)
endif()

# Compile Commands
execute_process(COMMAND
	${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json)
