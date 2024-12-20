#ifndef BOITATAH_UNIFORM_MANAGER_TEMP_HPP
#define BOITATAH_UNIFORM_MANAGER_TEMP_HPP

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
            void commitResource(Handle<GPUResource> resource, uint32_t frame_index);
            void freeResource(Handle<GPUResource> resource);


            Handle<GPUResource> create(const ResourceDescriptor& description);
            bool destroy(const Handle<GPUResource>& handle);

            //Pointer requires enough memory.
            bool readResourceData(Handle<GPUResource> handle, void* destinationPtr);
            void readResourceDataAsync(Handle<GPUResource> handle, void* destinationPtr);
            
            void commitAll();
            void cleanCommitQueue();
            
            GPUResource& getResource(Handle<GPUResource> &handle);
            Handle<BufferAddress>& getBufferAddress(Handle<GPUResource>& handle, uint32_t frame_index);
            ResourceMetaData& getResourceMetaData(Handle<GPUResource> &handle, uint32_t frame_index);

        private:
            vk::Vulkan* m_vulkan;
            std::weak_ptr<buffer::BufferManager> m_bufferManager;
            

            std::unique_ptr<Pool<GPUResource>> m_resourcePool;    

            void initialize_buffers(GPUResource& resource, const ResourceDescriptor &descriptor);
   
            void commitResource(GPUResource& resource);
            void commitResource(GPUResource& resource, uint32_t frame_index);
            
    };
};

#endif //BOITATAH_UNIFORM_MANAGER_HPP