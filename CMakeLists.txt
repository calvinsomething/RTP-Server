cmake_minimum_required(VERSION 3.22)

project(RTP-Server CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-pie -Wall -pthread)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	add_compile_options(-O2)
elseif(SANITIZE STREQUAL "Thread")
	set(COMPILE_OPTIONS "-fsanitize=thread")
else()
	set(COMPILE_OPTIONS "-fsanitize=address,undefined")
endif()

add_compile_options(${COMPILE_OPTIONS})
add_link_options(${COMPILE_OPTIONS})

set(MEDIA_DIRECTORY ${CMAKE_BINARY_DIR}/media)
add_compile_definitions("MEDIA_DIRECTORY=\"${MEDIA_DIRECTORY}\"")

# Source Files
set(SOURCE_DIR src)

add_executable(${CMAKE_PROJECT_NAME})

set(SOURCE_FILES
	main.cpp)

target_include_directories(${CMAKE_PROJECT_NAME}
	PRIVATE
		${CMAKE_SOURCE_DIR}/src)

foreach(path ${SOURCE_FILES})
	target_sources(${CMAKE_PROJECT_NAME} PRIVATE "${SOURCE_DIR}/${path}")
endforeach()

# Libraries
set(SERVER_FILES
	Server.cpp
	dispatch.cpp
	Connection.cpp
	Exception.cpp
	RTSPRequest.cpp
	RTSPResponse.cpp
	Session.cpp
	RTPPacket.cpp
	RTPTransport.cpp
	Stream.cpp
	Track.cpp
	UDP.cpp)

set(UTIL_FILES
	misc.cpp
	Defer.cpp
	RingBuffer.cpp)

foreach(lib Server;util)
	add_library(${lib})

	string(TOUPPER ${lib} lib_upper)

	target_include_directories(${lib}
		PRIVATE
			${CMAKE_SOURCE_DIR}/src)

	foreach(path ${${lib_upper}_FILES})
		target_sources(${lib} PRIVATE "${SOURCE_DIR}/${lib}/${path}")
	endforeach()

	target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${lib})
endforeach()

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
	avformat
	avcodec
	avutil)

# Tests
if (BUILD_TESTS)
	add_subdirectory(test)
endif()

# Compile Commands
execute_process(COMMAND
	${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json)

# Media
execute_process(COMMAND
	${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/media ${MEDIA_DIRECTORY})
