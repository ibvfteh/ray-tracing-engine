cmake_minimum_required(VERSION 3.0)

project(stbi)

set (CMAKE_POSITION_INDEPENDENT_CODE ON)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)


set(HEADER_FILES
		stb_image.cpp
		)

set(SOURCE_FILES
		stb_image.h
        )

add_library(stbi STATIC ${HEADER_FILES} ${SOURCE_FILES})
