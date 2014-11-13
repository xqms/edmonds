
cmake_minimum_required(VERSION 2.8)

project(edmonds)

# Default to release build
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "build type" FORCE)
endif()

# enable C++11, more warnings and optimizations applicable to this CPU
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -march=native")

# build with -O3 optimization even in RelWithDebInfo mode
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -g")

add_executable(edmonds
	graph.cpp
	edmonds.cpp
	main.cpp
)

# The verifier is not built by default
option(BUILD_VERIFIER "Build boost-based verifier" OFF)

if(BUILD_VERIFIER)
	find_package(Boost REQUIRED graph)

	add_executable(verifier
		verifier.cpp
		graph.cpp
	)
endif()
