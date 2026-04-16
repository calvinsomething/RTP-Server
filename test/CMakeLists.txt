set(TARGET_NAME RTP-Server-Tests)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_compile_definitions(BINARY_DIR="${CMAKE_BINARY_DIR}")

add_executable(${TARGET_NAME}
	unit/RTSP.cpp

	integration/RTSP.cpp
	integration/RTSPTest.cpp
)

target_link_libraries(${TARGET_NAME}
	GTest::gtest_main
	Server
	util)

target_include_directories(${TARGET_NAME}
	PRIVATE
		${CMAKE_SOURCE_DIR}/src)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})
