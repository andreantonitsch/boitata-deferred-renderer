#include "GPUResourceManager.hpp"
#include "../resources/GPUBuffer.hpp"

namespace boitatah{
    GPUResourceManager::GPUResourceManager(std::shared_ptr<vk::Vulkan>  vk_instance, std::shared_ptr<buffer::BufferManager> bufferManager)
    {
        m_vulkan = vk_instance;
        m_bufferManager = bufferManager;

        m_resourcePool = std::make_unique<GPUResourcePool>();


    }

    std::shared_ptr<buffer::BufferManager> GPUResourceManager::getBufferManager()
    {
        return std::shared_ptr(m_bufferManager);      
    }

    Handle<GPUBuffer> GPUResourceManager::create(const GPUBufferCreateDescription &description)
    {
        auto buffer = GPUBuffer(description, shared_from_this());
        return  m_resourcePool->set(buffer);
    }


    GPUBuffer&  GPUResourceManager::getResource(Handle<GPUBuffer> &handle)
    {
        return  m_resourcePool->get(handle);
    }

    // Handle<Geometry> GPUResourceManager::create(const ResourceCreateDescription<Geometry> &description)
    // {
    //     return Handle<Geometry>();
    // }

    // void GPUResourceManager::initialize_buffers(GPUResource &resource, const ResourceDescriptor &descriptor)
    // {
    //     auto buffer_manager = std::shared_ptr<BufferManager>(m_bufferManager);

    //     auto buffer_request = BufferReservationRequest{.request = descriptor.size,
    //                                                     .usage = descriptor.usage,
    //                                                     .sharing = descriptor.sharing,};
    //     auto buffer1 = buffer_manager->reserveBuffer(buffer_request);
    //     auto buffer2 = buffer_manager->reserveBuffer(buffer_request);

    //     resource.set_buffer(0, {buffer1, descriptor.size});
    //     resource.set_buffer(1, {buffer2, descriptor.size});
    // }
}
