#include "GPUResourceManager.hpp"

namespace boitatah{
    GPUResourceManager::GPUResourceManager(vk::Vulkan* vk_instance, std::shared_ptr<buffer::BufferManager> bufferManager)
    {
        m_vulkan = vk_instance;
        m_bufferManager = bufferManager;

        m_resourcePool = std::make_unique<Pool<GPUResource>>(PoolOptions{.size = 1 << 10, .dynamic = true, .name = "GPU Resource Manager Pool"});
    }

    void GPUResourceManager::update(Handle<GPUResource> handle, void *new_data, uint32_t new_size)
    {
        auto& resource = getResource(handle);

        resource.flag_update();
        resource.change_data({.data_size = new_size,.data=new_data,});

    }

    Handle<GPUResource> GPUResourceManager::create(const ResourceDescriptor &description)
    {
        GPUResource resource;
        resource.set_descriptor(description);

        initialize_buffers(resource, description);

        return m_resourcePool->set(resource);
    }

    bool GPUResourceManager::destroy(const Handle<GPUResource> &handle)
    {
        return false;
    }

    //TODO throws
    GPUResource &GPUResourceManager::getResource(Handle<GPUResource> &handle)
    {
        return m_resourcePool->get(handle);
    }

    void GPUResourceManager::initialize_buffers(GPUResource &resource, const ResourceDescriptor &descriptor)
    {
        auto buffer_manager = std::shared_ptr<BufferManager>(m_bufferManager);

        auto buffer_request = BufferReservationRequest{.request = descriptor.size,
                                                        .usage = descriptor.usage,
                                                        .sharing = descriptor.sharing,};
        auto buffer1 = buffer_manager->reserveBuffer(buffer_request);
        auto buffer2 = buffer_manager->reserveBuffer(buffer_request);

        resource.set_buffer(0, {buffer1, descriptor.size});
        resource.set_buffer(1, {buffer2, descriptor.size});
    }
}
