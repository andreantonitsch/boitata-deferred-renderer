#include "GPUResourceManager.hpp"

namespace boitatah{
    GPUResourceManager::GPUResourceManager(vk::Vulkan* vk_instance)
    {
    }

    Handle<GPUResource> GPUResourceManager::createResource(void *data, uint32_t size, DESCRIPTOR_TYPE type)
    {
        return Handle<GPUResource>();
    }

}
