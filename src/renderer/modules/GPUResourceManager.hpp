#ifndef BOITATAH_UNIFORM_MANAGER_HPP
#define BOITATAH_UNIFORM_MANAGER_HPP

#include <memory>

#include "../../vulkan/Vulkan.hpp"
#include "../../collections/Pool.hpp"
#include "../../buffers/Buffer.hpp"
#include "../../buffers/BufferManager.hpp"
#include "../../types/GPUResource.hpp"
#include "../../types/Geometry.hpp"

namespace boitatah
{

    class GPUResourceManager{

        public:
            GPUResourceManager(vk::Vulkan* vk_instance, std::shared_ptr<buffer::BufferManager> bufferManager); //contructor

            // Uniform Handling
            void update(Handle<GPUResource> resource, void *new_data, uint32_t new_size);
            
            void flagResource(Handle<GPUResource> resource);
            void commitResource(Handle<GPUResource> resource);
            void freeResource(Handle<GPUResource> resource);

            Handle<GPUResource> create(const ResourceDescriptor& description);
            //Handle<GPUResource> update(ResourceDescriptor& update);

            Handle<BufferAddress>& getBufferAddress(Handle<GPUResource>& handle);
            ResourceMetaData& getResourceMetaData(Handle<GPUResource> &handle);

            //Pointer requires enough memory.
            bool readResourceData(Handle<GPUResource> handle, void* destinationPtr);
            void readResourceDataAsync(Handle<GPUResource> handle, void* destinationPtr);
            
            void commitAll();
            void cleanCommitQueue();

        private:
            vk::Vulkan* m_vulkan;
            std::weak_ptr<buffer::BufferManager> m_bufferManager;
            
            std::unique_ptr<Pool<GPUResource>> m_resourcePool;
            
            //Pending updates
            Handle<GPUResource> createResource(void *data, uint32_t size, SHARING_MODE type);
            //std::vector<Handle<GPUResource>> pendingUpdate;

            
    };
};

#endif //BOITATAH_UNIFORM_MANAGER_HPP