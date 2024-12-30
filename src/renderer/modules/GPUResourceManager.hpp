#ifndef BOITATAH_RESOURCE_MANAGER_HPP
#define BOITATAH_RESOURCE_MANAGER_HPP

#include <memory>
#include <type_traits>

#include "../../vulkan/Vulkan.hpp"
#include "../../collections/Pool.hpp"
#include "../../buffers/Buffer.hpp"
#include "../../buffers/BufferManager.hpp"

#include "GPUResourcePool.hpp"

#include "../../types/GPUResource.hpp"
#include "../../command_buffers/CommandBufferWriter.hpp"

namespace boitatah
{
    template<typename T>
    using ResourceType = typename std::enable_if<std::is_base_of<GPUResource<T>, T>::value>::type;
    
    class GPUResourceManager{
        
        template <class T> friend class GPUResource;

        public:
            GPUResourceManager(vk::Vulkan* vk_instance, std::shared_ptr<buffer::BufferManager> bufferManager); //contructor

            // Uniform Handling
            template<typename ResourceType>
            void update(Handle<ResourceType> handle, ResourceUpdateDescription<ResourceType>& updateDescription );
            
            template<typename ResourceType>
            bool checkReady(Handle<ResourceType> handle, uint32_t frame_index);

            template<typename ResourceType>
            void flagResource(Handle<ResourceType> resource);

            template<typename ResourceType>
            void commitResource(Handle<ResourceType> resource);
            
            template<typename ResourceType>
            void commitResource(Handle<ResourceType> resource, uint32_t frame_index);
            
            template<typename ResourceType>
            void freeResource(Handle<ResourceType> resource);

            template<typename ResourceType>
            bool destroy(const Handle<ResourceType>& handle);

            //Pointer requires enough memory.
            template<typename ResourceType>
            bool readResourceData(Handle<ResourceType> handle, void* destinationPtr);
            
            template<typename ResourceType>
            void readResourceDataAsync(Handle<ResourceType> handle, void* destinationPtr);
            
            void commitAll();
            void cleanCommitQueue();
            
            template<typename ResourceType>
            ResourceType& getResource(Handle<ResourceType> &handle);
            
            template<typename ResourceType>
            ResourceMetaContent<ResourceType>& getResourceMetaData(Handle<ResourceType> &handle, uint32_t frame_index);
            
            template<typename ResourceType>
            ResourceGPUContent<ResourceType>& getResourceGPUData(Handle<ResourceType> &handle, uint32_t frame_index);

            std::shared_ptr<buffer::BufferManager> getBufferManager(){
                return std::shared_ptr(m_bufferManager);
            }

            Handle<GPUBuffer> create(const GPUBufferCreateDescription& description);
            Handle<Geometry> create(const GeometryCreateDescription& description);
        

        private:
            vk::Vulkan* m_vulkan;
            std::weak_ptr<buffer::BufferManager> m_bufferManager;
            
            std::unique_ptr<GPUResourcePool> m_resourcePool;


            // template<typename ResourceType>
            // void commitResource(ResourceType& resource);



            template<typename ResourceType>
            void commitResource(ResourceType& resource, uint32_t frame_index);

            template<typename ResourceType>
            ResourceType& getResource(Handle<ResourceType> handle);

            template<typename ResourceType>
            void setResource(ResourceType resource);
        };

        template <typename ResourceType>
        inline void GPUResourceManager::update(Handle<ResourceType> handle, ResourceUpdateDescription<ResourceType> &updateDescription)
        {
            auto& resource = getResource(resource);
            resource.self().update(updateDescription);
        }


        template <typename ResourceType>
        inline bool GPUResourceManager::checkReady(Handle<ResourceType> handle, uint32_t frame_index)
        {
            auto& resource = getResource(handle);
            return resource.self().ready_for_use(frame_index);
        }


        template <typename ResourceType>
        inline void GPUResourceManager::setResource(ResourceType resource)
        {
            return ResourceType();
        }

        template <typename ResourceType>
        ResourceType &GPUResourceManager::getResource(Handle<ResourceType> &handle)
        {
            return m_resourcePool->get(handle);
        }
};

#endif //BOITATAH_RESOURCE_MANAGER_TEMP_HPP