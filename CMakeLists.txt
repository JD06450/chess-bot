cmake_minimum_required(VERSION 4.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (UNIX)
	set(CMAKE_CXX_COMPILER "/usr/bin/g++")
	set(CMAKE_C_COMPILER "/usr/bin/gcc")
endif()

if (WIN32)
	set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe")
	set(CMAKE_C_COMPILER "C:/msys64/mingw64/bin/gcc.exe")

	if (CMAKE_GENERATOR MATCHES "Ninja")
		set(CMAKE_MAKE_PROGRAM "C:/msys64/mingw64/bin/ninja.exe")
	endif()
endif()

# if (CMAKE_BUILD_TYPE MATCHES "Debug")
# 	add_compile_options(-pg)
# 	add_link_options(-pg)
# endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

# find_library(TBB REQUIRED)

project(chess_bot)

file(GLOB_RECURSE sources src/main/*.cpp)
file(GLOB_RECURSE source_tests src/tests/*.cpp)

add_executable(bot ${sources})

target_include_directories(bot PRIVATE include/main PRIVATE cxxopts/include)
target_link_libraries(bot PRIVATE tbb)

list(REMOVE_ITEM sources "${CMAKE_SOURCE_DIR}/src/main/main.cpp")

add_executable(bot_tests ${source_tests} ${sources})

target_include_directories(bot_tests PRIVATE include/tests PRIVATE include/main PRIVATE cxxopts/include)
target_link_libraries(bot_tests PRIVATE tbb)