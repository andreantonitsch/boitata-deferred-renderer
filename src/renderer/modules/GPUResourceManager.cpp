#include "GPUResourceManager.hpp"

namespace boitatah{
    GPUResourceManager::GPUResourceManager(vk::Vulkan* vk_instance, std::shared_ptr<buffer::BufferManager> bufferManager)
    {
    }

    Handle<GPUResource> GPUResourceManager::create(const ResourceDescriptor &description)
    {
        return Handle<GPUResource>();
    }

    Handle<GPUResource> GPUResourceManager::createResource(void *data, uint32_t size, SHARING_MODE type)
    {
        return Handle<GPUResource>();
    }

}
