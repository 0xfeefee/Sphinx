##
## Sphinx build
##
cmake_minimum_required(VERSION 3.20)
project(Sphinx
	VERSION 0.6.0
	LANGUAGES C CXX
)

## Basic configuration and variables:
set(CMAKE_C_STANDARD   17)
set(CMAKE_CXX_STANDARD 17)
set(SRC 			   "${CMAKE_CURRENT_LIST_DIR}/../src")
set(VENDOR             "${CMAKE_CURRENT_LIST_DIR}/../vendor")
set(TESTS              "${CMAKE_CURRENT_LIST_DIR}/../tests")

## Check release mode:
if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_compile_definitions(PROJECT_BUILD_DEBUG=1)
else()
	add_compile_definitions(PROJECT_BUILD_RELEASE=1)
endif()


## Add: Dear ImGui
add_library(lib_sphinx_ui
	STATIC

	# ImGui:
	${VENDOR}/imgui-docking-1.91.6/imgui.cpp
	${VENDOR}/imgui-docking-1.91.6/imgui_draw.cpp
	${VENDOR}/imgui-docking-1.91.6/imgui_widgets.cpp
	${VENDOR}/imgui-docking-1.91.6/imgui_tables.cpp
	${VENDOR}/imgui-docking-1.91.6/imgui_demo.cpp
	${VENDOR}/imgui-docking-1.91.6/imgui_impl_opengl3.cpp
	${VENDOR}/imgui-docking-1.91.6/imgui_impl_glfw.cpp

	# File dialog:
	${VENDOR}/imgui-docking-1.91.6/file_dialog/ImGuiFileDialog.cpp

	# gl3w:
	${VENDOR}/gl3w-master/src/gl3w.c
)

target_include_directories(lib_sphinx_ui
	PUBLIC

	${VENDOR}/imgui-docking-1.91.6
	${VENDOR}/dirent/include
	${VENDOR}/gl3w-master/include
	${VENDOR}/Win64/glfw-3.4/include
)

find_package(OpenGL REQUIRED)

target_link_libraries(lib_sphinx_ui
	${VENDOR}/Win64/glfw-3.4/lib/glfw3.lib
	OpenGL::GL
)

set(DLLS_TO_COPY
	${VENDOR}/Win64/glfw-3.4/lib/glfw3.dll
)

## Create an executable:
add_executable(${PROJECT_NAME}
	${SRC}/main.cpp
	${SRC}/sphinx/AES.cpp
	${SRC}/sphinx/GUI.cpp
	${SRC}/sphinx/Image.cpp
	${SRC}/sphinx/PNG_Image.cpp
	${SRC}/sphinx/Image_Manager.cpp
	${SRC}/scene/Password_Manager.cpp
	${SRC}/util/Thread_Pool.cpp
	${SRC}/util/Circular_Vector.cpp
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
	${SRC}
	${VENDOR}/stb
)

target_precompile_headers(${PROJECT_NAME}
	PRIVATE
	${SRC}/base.pch.hpp
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
	lib_sphinx_ui
)

## Make sure to enable AES intrinsics:
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(CMAKE_CXX_COMPILER_FLAGS_DEBUG "-Wextra -Werror")
	set(CMAKE_CXX_COMPILER_FLAGS_RELEASE "-Wall -Wextra -Werror")
	target_compile_options(${PROJECT_NAME}
		PRIVATE
		-maes
	)
else()
	message(FATAL_ERROR "Untested compiler!")
endif()

## Compile definitions:
add_compile_definitions(PROJECT_NAME="${PROJECT_NAME}")

if (WIN32)
	target_compile_definitions(${PROJECT_NAME}
		PRIVATE
		PROJECT_PLATFORM_WIN64
	)
endif()
