#include "GPUResourceManager.hpp"

namespace boitatah{
    GPUResourceManager::GPUResourceManager(vk::Vulkan* vk_instance, std::shared_ptr<buffer::BufferManager> bufferManager)
    {
        m_vulkan = vk_instance;
        m_bufferManager = bufferManager;

        m_resourcePool = std::make_unique<Pool<GPUResource>>(PoolOptions{.size = 1 << 10, .dynamic = true, .name = "GPU Resource Manager Pool"});
    }

    void GPUResourceManager::update(Handle<GPUResource> resource, void *new_data, uint32_t new_size)
    {

    }

    Handle<GPUResource> GPUResourceManager::create(const ResourceDescriptor &description)
    {
        GPUResource resource;
        

        return Handle<GPUResource>();
    }

    Handle<GPUResource> GPUResourceManager::createResource(void *data, uint32_t size, SHARING_MODE type)
    {
        return Handle<GPUResource>();
    }

}
