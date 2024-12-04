##
## Sphinx build
##
cmake_minimum_required(VERSION 3.20)
project(Sphinx
	VERSION 0.1.0
	LANGUAGES C CXX
)

## Basic configuration and variables:
set(CMAKE_CXX_STANDARD 17)
set(SRC 			   "../src")

## Check release mode:
if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_compile_definitions(PROJECT_BUILD_DEBUG=1)
else()
	add_compile_definitions(PROJECT_BUILD_RELEASE=1)
endif()

## Create an executable:
add_executable(${PROJECT_NAME}
	${SRC}/main.cpp
	${SRC}/sphinx/AES.cpp
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
	${SRC}
)

target_precompile_headers(${PROJECT_NAME}
	PRIVATE
	${SRC}/base.pch.hpp
)

## Make sure to enable AES intrinsics:
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(${PROJECT_NAME}
		PRIVATE
		-maes
	)
else()
	message(FATAL_ERROR "Untested compiler!")
endif()
