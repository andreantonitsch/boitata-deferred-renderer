#pragma once

#include <memory>
#include <type_traits>

#include "../../vulkan/Vulkan.hpp"
#include "../../command_buffers/CommandBufferWriter.hpp"
#include "../../vulkan/VkCommandBufferWriter.hpp" 

#include "../../collections/Pool.hpp"
#include "../../buffers/Buffer.hpp"
#include "../../buffers/BufferManager.hpp"
#include "../resources/ResourceStructs.hpp"
#include "GPUResourcePool.hpp"
#include <renderer/resources/GPUBuffer.hpp>
#include <types/Texture.hpp>
#include <types/Geometry.hpp>

#include "../../command_buffers/CommandBufferWriter.hpp"

namespace boitatah
{
    // class GPUBuffer;
    // class Geometry;
    // class RenderTexture;

    template<template <typename > class DerivedResource, typename Resource>
    class GPUResource;

    class GPUResourceManager : public std::enable_shared_from_this<GPUResourceManager>
    {
        
        template<template <typename > class T, typename Y> friend class GPUResource;
        
        public:
            GPUResourceManager(std::shared_ptr<vk::Vulkan> vk_instance,
                               std::shared_ptr<buffer::BufferManager> bufferManager,
                               std::shared_ptr<vk::VkCommandBufferWriter> commandBufferWriter); //contructor


            vk::VkCommandBufferWriter& getCommandBufferWriter(){
                return *m_commandBufferWriter;
            }
            
            template<typename ResourceType>
            bool checkReady(Handle<ResourceType> handle, uint32_t frame_index);

            template<typename ResourceType>
            void flagResource(Handle<ResourceType> resource);

            template<typename ResourceType>
            void forceCommitResource(Handle<ResourceType> resource);
            
            template<typename ResourceType>
            void forceCommitResource(Handle<ResourceType> resource, uint32_t frame_index);
            
            template<typename ResourceType>
            void freeResource(Handle<ResourceType> resource);

            //Pointer requires enough memory.
            template<typename ResourceType>
            bool readResourceData(Handle<ResourceType> handle, void* destinationPtr);
            
            template<typename ResourceType>
            void readResourceDataAsync(Handle<ResourceType> handle, void* destinationPtr);
            
            void commitAll(uint32_t frameIndex);
            void cleanCommitQueue();

            void beginCommitCommands();

            template<typename ResourceType>
            void commitResourceCommand(Handle<ResourceType> handle, uint32_t frameIndex);

            void submitCommitCommands();

            bool checkTransfers();
            void waitForTransfers();

            std::shared_ptr<buffer::BufferManager> getBufferManager();

            Handle<GPUBuffer> create(const GPUBufferCreateDescription& description);
            Handle<Geometry> create(const GeometryCreateDescription& description);
            Handle<RenderTexture> create(const TextureCreateDescription& description);

            template <typename ResourceType>
            void destroy(const Handle<ResourceType>& handle);

            template <typename ResourceType>
            inline ResourceType& getResource(Handle<ResourceType> handle)
            {
                return m_resourcePool->get(handle);
            }


        private:
            std::shared_ptr<vk::Vulkan> m_vulkan;
            std::shared_ptr<buffer::BufferManager> m_bufferManager;
            
            std::unique_ptr<GPUResourcePool> m_resourcePool;
            std::shared_ptr<vk::VkCommandBufferWriter> m_commandBufferWriter;

            void commitGeometryData(Geometry& geo);

        };


        template <typename ResourceType>
        inline bool GPUResourceManager::checkReady(Handle<ResourceType> handle, uint32_t frame_index)
        {
            auto& resource = getResource(handle);
            return resource.self().ready_for_use(frame_index);
        }

        template <typename ResourceType>
        inline void GPUResourceManager::forceCommitResource(Handle<ResourceType> resource)
        {
            forceCommitResource(resource, 0);
        }

        template <typename ResourceType>
        inline void GPUResourceManager::forceCommitResource(Handle<ResourceType> resource, uint32_t frame_index)
        {
            if(!checkTransfers()) //if fence is signaled
                waitForTransfers();
            beginCommitCommands();
            commitResourceCommand(resource, frame_index);
            submitCommitCommands();
        }

        template <typename ResourceType>
        inline void GPUResourceManager::commitResourceCommand(Handle<ResourceType> handle, uint32_t frameIndex)
        {
            auto& resource = m_resourcePool->get(handle);
            resource.commit(frameIndex, getCommandBufferWriter());
        }

        template <typename ResourceType>
        inline void GPUResourceManager::destroy(const Handle<ResourceType> &handle)
        {
            auto& resource = getResource(handle);
            resource.release();
            m_resourcePool->clear(handle);
        }


};
