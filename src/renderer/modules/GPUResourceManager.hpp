#pragma once

#include <memory>
#include <type_traits>

#include <vulkan/Vulkan.hpp>
#include <command_buffers/CommandBufferWriter.hpp>
#include <vulkan/VkCommandBufferWriter.hpp>
#include <renderer/modules/ImageManager.hpp>
#include <collections/Pool.hpp>
#include <buffers/Buffer.hpp>
#include <buffers/BufferManager.hpp>
#include <renderer/resources/ResourceStructs.hpp>
#include "GPUResourcePool.hpp"
#include <renderer/resources/GPUBuffer.hpp>
#include <types/Texture.hpp>
#include <renderer/resources/Geometry.hpp>

#include <command_buffers/CommandBufferWriter.hpp>

namespace boitatah
{

    template<template <typename > class DerivedResource, typename Resource>
    class GPUResource;

    class GPUResourceManager : public std::enable_shared_from_this<GPUResourceManager>
    {
        
        template<template <typename > class T, typename Y> friend class GPUResource;
        
        public:
            GPUResourceManager(std::shared_ptr<vk::Vulkan> vk_instance,
                               std::shared_ptr<buffer::BufferManager> bufferManager,
                               std::shared_ptr<ImageManager> imageManager,
                               uint32_t buffer_writer_count); //contructor

            vk::VkCommandBufferWriter& getCurrentBufferWriter(){
                return *m_buffer_writers[m_current_writer];
            }
            
            std::shared_ptr<buffer::BufferManager> getBufferManager();

            ImageManager& getImageManager();


            template<typename ResourceType>
            bool checkReady( Handle<ResourceType>    handle, 
                             uint32_t                frame_index );

            template<typename ResourceType>
            void forceCommitResource( Handle<ResourceType>      resource );
            
            template<typename ResourceType>
            void forceCommitResource( Handle<ResourceType>      resource, 
                                      uint32_t                  frame_index );
            
            //Pointer requires enough memory.
            template<typename ResourceType>
            bool readResourceData( Handle<ResourceType>     handle, 
                                   void*                    destinationPtr );
            
            template<typename ResourceType>
            void readResourceDataAsync( Handle<ResourceType>    handle,
                                        void*                   destinationPtr );
            
            //void commitAll( uint32_t frameIndex );

            void beginCommitCommands();

            template<typename ResourceType>
            void commitResourceCommand( Handle<ResourceType>    handle,
                                        uint32_t                frameIndex );

            void submitCommitCommands();

            void beginNewCommitCommands();

            template<typename ResourceType>
            void commitNewResourceCommand( Handle<ResourceType>    handle,
                                        uint32_t                frameIndex );

            void submitNewCommitCommands();

            bool checkTransfers();
            void waitForTransfers();


            Handle<GPUBuffer>       create( const GPUBufferCreateDescription&    description );
            Handle<Geometry>        create( const GeometryCreateDescription&     description );
            Handle<RenderTexture>   create( const TextureCreateDescription&      description );

            template <typename ResourceType>
            void destroy( const Handle<ResourceType>& handle );

            template <typename ResourceType>
            inline ResourceType& getResource( Handle<ResourceType> handle )
            {
                return m_resourcePool->get(handle);
            }

            //can only be using when commiting commands
            template <typename ResourceType>
            inline ResourceType& getCommitResource( Handle<ResourceType>&   handle, 
                                                    uint32_t                frame_index)
            {
                return m_resourcePool->get(handle)
                            .get_content_commit_update(frame_index,
                                                       getCurrentBufferWriter());
            }

            //ACCESS DATA is the hot data for rendering
            template <typename ResourceType>
            inline ResourceTraits<ResourceType>::RenderData getResourceAccessData(
                                                            Handle<ResourceType>    handle,
                                                            uint32_t                frame_index )
            {
                return m_resourcePool->get(handle).get_render_data(frame_index);
            }
            //ACCESS DATA is the hot data for rendering
            //can only be used when writing transfers
            template <typename ResourceType>
            inline ResourceTraits<ResourceType>::RenderData getCommitResourceAccessData(
                                                            Handle<ResourceType>        handle,
                                                            uint32_t                    frame_index )
            {
                return m_resourcePool->get(handle)
                        .get_render_data_commit_update( 
                            frame_index,
                            getCurrentBufferWriter());
            }

        private:
            std::shared_ptr<vk::Vulkan>             m_vulkan;
            
            std::shared_ptr<buffer::BufferManager>  m_bufferManager;
            std::shared_ptr<ImageManager>           m_imageManager;
            
            std::unique_ptr<GPUResourcePool>            m_resourcePool;
            std::vector<std::shared_ptr<vk::VkCommandBufferWriter>> m_buffer_writers;
            uint32_t m_current_writer;
            void commitGeometryData( Geometry& geo );

            bool recording = false;

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
            forceCommitResource(resource, 1);
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
            resource.commit(frameIndex, getCurrentBufferWriter());
        }

        template <typename ResourceType>
        inline void GPUResourceManager::destroy(const Handle<ResourceType> &handle)
        {
            auto& resource = getResource(handle);
            resource.release();
            m_resourcePool->clear(handle);
        }


};
