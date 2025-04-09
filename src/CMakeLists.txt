cmake_minimum_required(VERSION 3.9.1)

set(LIBRARY_OUTPUT_PATH ${OUTPUT_FOLDER}/libs)

set (LIBRARY_INCLUDE_PATH ./)
set (UTIL_INCLUDE_PATH ./utils/)

set(LIB_DIR_SOURCES
            collections/PartitionList.cpp
            buffers/BufferAllocator.cpp
            buffers/Buffer.cpp
            buffers/BufferManager.cpp
            renderer/resources/builders/GeometryBuilder.cpp
            renderer/modules/ImageManager.cpp
            renderer/modules/RenderTargetManager.cpp
            renderer/modules/MaterialManager.cpp
            renderer/modules/StageBaseMaterialManager.cpp
            renderer/modules/GPUResourceManager.cpp
            renderer/modules/GPUResourcePool.cpp
            types/Texture.cpp
            types/Lights.cpp
            renderer/resources/GPUBuffer.cpp
            renderer/modules/Swapchain.cpp
            renderer/modules/Camera.cpp
            renderer/modules/BackBuffer.cpp
            renderer/Renderer.cpp
            renderer/modules/Camera.cpp
            vulkan/DescriptorSetManager.cpp
            vulkan/DescriptorSetTree.cpp
            vulkan/Vulkan.cpp
            vulkan/Window.cpp
)

add_library(boitatah STATIC ${LIB_DIR_SOURCES})

target_include_directories(boitatah PUBLIC ${LIBRARY_INCLUDE_PATH} ${UTIL_INCLUDE_PATH} ${SPIRV_Headers_SOURCE_DIR}/include ${THIRD_PARTY_INCLUDE_PATH})
target_compile_definitions(boitatah PUBLIC SPIRV_REFLECT_USE_SYSTEM_SPIRV_H)
target_compile_definitions(boitatah PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)