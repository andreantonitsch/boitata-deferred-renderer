cmake_minimum_required(VERSION 3.9.1)

set(LIBRARY_OUTPUT_PATH ${OUTPUT_FOLDER}/libs)

set(LIB_DR_SOURCES
            renderer/Renderer.cpp
            
)

add_library(velly STATIC ${LIB_DR_SOURCES})

