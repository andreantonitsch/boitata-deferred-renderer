#include "GPUResourceManager.hpp"


namespace boitatah{
    GPUResourceManager::GPUResourceManager(vk::Vulkan* vk_instance, std::shared_ptr<buffer::BufferManager> bufferManager)
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
        
        return Handle<GPUBuffer>();
        //return m_resourcePool->set(new GPUBuffer(std::static_cast<GPUBufferCreateDescription>(description) ));
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
