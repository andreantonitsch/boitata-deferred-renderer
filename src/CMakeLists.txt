cmake_minimum_required(VERSION 3.9.1)

set(LIBRARY_OUTPUT_PATH ${OUTPUT_FOLDER}/libs)


set(LIB_DIR_SOURCES
            collections/PartitionList.cpp

            backends/vulkan/Vulkan.cpp
            backends/vulkan/Window.cpp

            buffers/BufferAllocator.cpp
            buffers/Buffer.cpp
            buffers/BufferManager.cpp

            renderer/resources/builders/GeometryBuilder.cpp
            renderer/resources/Texture.cpp
            renderer/resources/GPUBuffer.cpp
            renderer/resources/Geometry.cpp

            renderer/modules/ImageManager.cpp
            renderer/modules/RenderTargetManager.cpp
            renderer/modules/MaterialManager.cpp
            renderer/modules/StageBaseMaterialManager.cpp
            renderer/modules/GPUResourceManager.cpp
            renderer/modules/GPUResourcePool.cpp
            renderer/modules/Swapchain.cpp
            renderer/modules/Camera.cpp
            renderer/modules/BackBuffer.cpp
            renderer/modules/Camera.cpp
            renderer/modules/DescriptorSetManager.cpp
            renderer/modules/DescriptorSetTree.cpp

            lights/Lights.cpp
            
            renderer/Renderer.cpp
            )
            
add_library(boitatah STATIC ${LIB_DIR_SOURCES})

target_link_libraries(boitatah PUBLIC boitatah_includes)

target_include_directories(boitatah PUBLIC ${PRIVATE_INCLUDE_PATH} ${SPIRV_Headers_SOURCE_DIR}/include ${THIRD_PARTY_INCLUDE_PATH})

target_compile_definitions(boitatah PUBLIC SPIRV_REFLECT_USE_SYSTEM_SPIRV_H)
target_compile_definitions(boitatah PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_compile_definitions(boitatah PUBLIC GLM_ENABLE_EXPERIMENTAL )