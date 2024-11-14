cmake_minimum_required(VERSION 3.9.1)

set(LIBRARY_OUTPUT_PATH ${OUTPUT_FOLDER}/libs)

set(LIB_DR_SOURCES
            collections/PartitionList.cpp
            buffers/BufferAllocator.cpp
            buffers/Buffer.cpp
            buffers/BufferManager.cpp
            renderer/modules/Swapchain.cpp
            renderer/modules/Camera.cpp
            renderer/modules/BackBuffer.cpp
            renderer/Renderer.cpp
            renderer/modules/Camera.cpp
            renderer/modules/Window.cpp
            vulkan/Vulkan.cpp
)

add_library(boitatah STATIC ${LIB_DR_SOURCES})

