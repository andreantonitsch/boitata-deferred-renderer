cmake_minimum_required(VERSION 3.9.1)

set(LIBRARY_OUTPUT_PATH ${OUTPUT_FOLDER}/libs)

set(LIB_DR_SOURCES
            collections/PartitionList.cpp
            collections/BufferAllocator.cpp
            types/Buffer.cpp
            types/Swapchain.cpp
            renderer/Camera.cpp
            renderer/BackBuffer.cpp
            renderer/Renderer.cpp
            renderer/Camera.cpp
            renderer/Window.cpp
            vulkan/Vulkan.cpp
)

add_library(boitatah STATIC ${LIB_DR_SOURCES})

