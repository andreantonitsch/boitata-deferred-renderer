#include  <boitatah/modules/GPUResourceManager.hpp>

#include <boitatah/resources/GPUResource.hpp>
#include <boitatah/resources/GPUBuffer.hpp>
#include <boitatah/modules/GPUResourcePool.hpp>


namespace boitatah{
    GPUResourceManager::GPUResourceManager( std::shared_ptr<vk::VulkanInstance>  vk_instance,
                                            std::shared_ptr<buffer::BufferManager> bufferManager,
                                            std::shared_ptr<ImageManager> imageManager,
                                            uint32_t buffer_writer_count = 10)
                                            : 
                                              m_vulkan(vk_instance),
                                              m_bufferManager(bufferManager),
                                              m_imageManager(imageManager),
                                              m_resourcePool(std::make_unique<GPUResourcePool>())
    { 
        for(int i = 0; i < buffer_writer_count; i++)
        {
            auto buffer_writer = std::make_shared<VkCommandBufferWriter>(m_vulkan);
            buffer_writer->set_commandbuffer(m_vulkan->allocate_commandbuffer({.count = 1,
                                                        .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                                                        .type = COMMAND_BUFFER_TYPE::TRANSFER}).buffer);
            buffer_writer->set_fence(m_vulkan->create_fence(true));
            buffer_writer->set_signal(m_vulkan->create_semaphore());
            m_buffer_writers.push_back(buffer_writer);
        }
    }

    GPUResourceManager::~GPUResourceManager()
    {
        for(int i = 0; i < m_buffer_writers.size(); i++){
            auto& buffer_writer = m_buffer_writers[i];
            m_vulkan->destroy_fence(*buffer_writer->get_fence());
            m_vulkan->destroy_semaphore(*buffer_writer->get_signal());
        }
    }

    void GPUResourceManager::beginCommitCommands()
    {
        
        recording = true;
        m_current_writer = (m_current_writer+1u) % m_buffer_writers.size();

        auto& buffer_writer = m_buffer_writers[m_current_writer];
        
        buffer_writer->waitForTransfers();
        buffer_writer->reset({});
        buffer_writer->begin({});
        
    }
    
    void GPUResourceManager::submitCommitCommands()
    {
        auto& buffer_writer = m_buffer_writers[m_current_writer];
        
        buffer_writer->submit({
            .submitType = COMMAND_BUFFER_TYPE::TRANSFER,
            .signal = true
        });
        recording = false;
    }

    bool GPUResourceManager::checkTransfers()
    {
        return m_buffer_writers[m_current_writer]->checkTransfers();
    }

    void GPUResourceManager::waitForTransfers()
    {
        m_buffer_writers[m_current_writer]->waitForTransfers();
    }

    std::shared_ptr<buffer::BufferManager> GPUResourceManager::getBufferManager()
    {
        return m_bufferManager;
    }

    ImageManager& GPUResourceManager::getImageManager()
    {
        return *m_imageManager;
    }

    Handle<GPUBuffer> GPUResourceManager::create(const GPUBufferCreateDescription &description)
    {
        auto buffer = GPUBuffer(description, shared_from_this());
        auto added = m_resourcePool->set(buffer);
        //if(!added) std::cout << "failed to add to resource pool after creation" << std::endl;
        return  added;
    };

    Handle<Geometry> GPUResourceManager::create(const GeometryCreateDescription& description)
    {

        Geometry geo(shared_from_this());
        for(auto& bufferDesc : description.bufferData)
        {

            Handle<GPUBuffer> bufferHandle;
            if(bufferDesc.data_type == GEO_DATA_TYPE::Ptr)
            {
                uint32_t data_size = static_cast<uint32_t>(bufferDesc.vertexCount * bufferDesc.vertexSize);
                bufferHandle = create(GPUBufferCreateDescription{
                    .size = data_size,
                    .usage = BUFFER_USAGE::VERTEX,
                    .sharing_mode = SHARING_MODE::EXCLUSIVE,
                });
                auto& buffer = getResource(bufferHandle);
                buffer.setStrideCount(bufferDesc.vertexSize, bufferDesc.vertexCount);


                buffer.copyData(bufferDesc.vertexDataPtr, data_size);
                geo.addOwnedBuffer(bufferHandle, bufferDesc.buffer_type);
            }

            if(bufferDesc.data_type == GEO_DATA_TYPE::GPUBuffer){
                bufferHandle = bufferDesc.buffer;
                geo.addExternalBuffer(bufferHandle, bufferDesc.buffer_type);
            }

        }
        if (description.indexData.count != 0)
        {

            if(description.indexData.data_type == GEO_DATA_TYPE::Ptr){
                uint32_t data_size = static_cast<uint32_t>(description.indexData.count  * sizeof(uint32_t));
                auto bufferHandle = create(GPUBufferCreateDescription{
                    .size = data_size,
                    .usage = BUFFER_USAGE::INDEX,
                    .sharing_mode = SHARING_MODE::EXCLUSIVE,
                });
                auto& buffer = getResource(bufferHandle);
                buffer.copyData(description.indexData.dataPtr, data_size);
                geo.indexBuffer = bufferHandle;
                geo.indiceCount = description.indexData.count;
            }
        }
        geo.vertexInfo = description.vertexInfo;
        //commitGeometryData(geo);
        std::cout << "commited" << std::endl;
        return m_resourcePool->set(geo);
    }

    Handle<RenderTexture> GPUResourceManager::create(const TextureCreateDescription &description)
    {
        RenderTexture tex(description, shared_from_this());
        
        return m_resourcePool->set(tex);

    }

    void GPUResourceManager::commitGeometryData(Geometry &geo)
    {
        waitForTransfers();
        // beginCommitCommands();
        // for(auto& buffer : geo.m_buffers)
        // {
        //     commitResourceCommand(buffer, 0);
        //     commitResourceCommand(buffer, 1);    
        // }

        //commitResourceCommand(geo.indexBuffer, 0);
        //commitResourceCommand(geo.indexBuffer, 1);
        //submitCommitCommands();
    }
}
