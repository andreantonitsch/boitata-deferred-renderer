#include "GPUResourceManager.hpp"
#include "../resources/GPUBuffer.hpp"
#include "../resources/GPUResource.hpp"
#include <types/Geometry.hpp>
#include "../../types/BttEnums.hpp"

namespace boitatah{
    GPUResourceManager::GPUResourceManager( std::shared_ptr<vk::Vulkan>  vk_instance,
                                            std::shared_ptr<buffer::BufferManager> bufferManager,
                                            std::shared_ptr<vk::VkCommandBufferWriter> commandBufferWriter)
                                            : m_commandBufferWriter(commandBufferWriter),
                                              m_vulkan(vk_instance),
                                              m_bufferManager(bufferManager),
                                              m_resourcePool(std::make_unique<GPUResourcePool>())
    { }

    void GPUResourceManager::commitAll(uint32_t frameIndex)
    {

    }

    void GPUResourceManager::beginCommitCommands()
    {
        m_commandBufferWriter->reset({});
        m_commandBufferWriter->begin({});
    }

    void GPUResourceManager::submitCommitCommands()
    {
        //std::cout << "submiting transfer commands" << std::endl;
        m_commandBufferWriter->submit({
            .submitType = COMMAND_BUFFER_TYPE::TRANSFER,
        });
    }

    bool GPUResourceManager::checkTransfers()
    {
        return m_commandBufferWriter->checkTransfers();
    }

    void GPUResourceManager::waitForTransfers()
    {
        m_commandBufferWriter->waitForTransfers();
    }

    std::shared_ptr<buffer::BufferManager> GPUResourceManager::getBufferManager()
    {
        return m_bufferManager;
    }

    Handle<GPUBuffer> GPUResourceManager::create(const GPUBufferCreateDescription &description)
    {
        auto buffer = GPUBuffer(description, shared_from_this());
        auto added = m_resourcePool->set(buffer);
        if(!added) std::cout << "failed to add to resource pool after creation" << std::endl;
        return  added;
    };

    Handle<Geometry> GPUResourceManager::create(const GeometryCreateDescription& description)
    {

        Geometry geo{};
        for(auto& bufferDesc : description.bufferData)
        {

            Handle<GPUBuffer> bufferHandle;
            if(bufferDesc.type == GEO_BUFFER_TYPE::Ptr)
            {
                uint32_t data_size = static_cast<uint32_t>(bufferDesc.vertexCount * bufferDesc.vertexSize);
                bufferHandle = create(GPUBufferCreateDescription{
                    .size = data_size,
                    .usage = BUFFER_USAGE::TRANSFER_DST_VERTEX,
                    .sharing_mode = SHARING_MODE::EXCLUSIVE,
                });
                auto& buffer = getResource(bufferHandle);
                buffer.setStrideCount(bufferDesc.vertexSize, bufferDesc.vertexCount);
                buffer.copyData(bufferDesc.vertexDataPtr, data_size);
                geo.addOwnedBuffer(bufferHandle);
            }

            if(bufferDesc.type == GEO_BUFFER_TYPE::GPUBuffer){
                bufferHandle = bufferDesc.buffer;
                geo.addExternalBuffer(bufferHandle);
            }

        }

        if (description.indexData.count != 0)
        {
            uint32_t data_size = static_cast<uint32_t>(description.indexData.count  * sizeof(uint32_t));
            auto bufferHandle = create(GPUBufferCreateDescription{
                .size = data_size,
                .usage = BUFFER_USAGE::TRANSFER_DST_INDEX,
                .sharing_mode = SHARING_MODE::EXCLUSIVE,
            });
            auto& buffer = getResource(bufferHandle);
            buffer.copyData(description.indexData.dataPtr, data_size);
            geo.indexBuffer = bufferHandle;
            geo.indiceCount = description.indexData.count;
        }

        geo.vertexInfo = description.vertexInfo;
        geo.indiceCount = description.indexData.count;
        commitGeometryData(geo);

        return m_resourcePool->set(geo);
    }

    void GPUResourceManager::commitGeometryData(Geometry &geo)
    {
        waitForTransfers();
        beginCommitCommands();
        for(auto& buffer : geo.m_buffers)
        {
            commitResourceCommand(buffer, 0);
            commitResourceCommand(buffer, 1);    
        }

        commitResourceCommand(geo.indexBuffer, 0);
        commitResourceCommand(geo.indexBuffer, 1);
        submitCommitCommands();
    }
}
