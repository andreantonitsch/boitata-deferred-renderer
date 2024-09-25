#ifndef BOITATAH_UNIFORM_MANAGER_HPP
#define BOITATAH_UNIFORM_MANAGER_HPP

#include "../../vulkan/Vulkan.hpp"
#include "../collections/Pool.hpp"
#include "../buffers/Buffer.hpp"
#include "../buffers/BufferManager.hpp"
#include "../types/GPUResource.hpp"

namespace boitatah
{
    class GPUResourceManager{

        public:
        GPUResourceManager(vk::Vulkan* vk_instance); //contructor

        // Uniform Handling
        Handle<GPUResource> createResource(void *data, uint32_t size, DESCRIPTOR_TYPE type);
        void updateResource(void *new_data, uint32_t new_size);
        
        void flagResource(Handle<GPUResource> uniform);
        void commitResource(Handle<GPUResource> uniform);
        void freeResource(Handle<GPUResource> uniform);

        

        private:
            vk::Vulkan* m_vulkan;
            BufferManager bufferManager;
            
            std::vector<Pool<GPUResource>> pools;
            Pool<GPUResource> uniformPool = Pool<GPUResource>({.size = 1<<16, .name = "uniforms pool"});


    };
};

#endif //BOITATAH_UNIFORM_MANAGER_HPP