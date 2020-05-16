cmake_minimum_required(VERSION 3.0)

project(imgui)

set (CMAKE_POSITION_INDEPENDENT_CODE ON)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)


set(HEADER_FILES
		imgui.cpp
		imgui_draw.cpp
		imgui_widgets.cpp
		imgui_demo.cpp
    )

set(SOURCE_FILES
		imconfig.h
		imgui.h
		imgui_internal.h
		imstb_rectpack.h
		imstb_textedit.h
		imstb_truetype.h
    )

add_library(imgui STATIC ${HEADER_FILES} ${SOURCE_FILES})
