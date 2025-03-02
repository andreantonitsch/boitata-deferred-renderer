#include "GPUBuffer.hpp"
#include "../modules/GPUResourceManager.hpp"
#include  "../../buffers/BufferManager.hpp"

namespace boitatah{

    buffer::BufferAccessData GPUBuffer::GetRenderData(uint32_t frame_index)
    {
        auto& content = get_content(frame_index);
        std::shared_ptr<GPUResourceManager> manager(m_manager);
        auto addr = manager->getBufferManager()->getBufferAccessData(content.buffer);

        return addr;
    }

    void GPUBuffer::copyData(const void *data, uint32_t length)
    {
        
        dirty = true;
        //Stages a transfer
        if(m_descriptor.sharing == SHARING_MODE::EXCLUSIVE){
            
            auto manager = std::shared_ptr(m_manager); 
            auto bufferManager = manager->getBufferManager();


            if(!stagingBuffer){
                stagingBuffer = bufferManager->reserveBuffer({
                    .request = size,
                    .usage = BUFFER_USAGE::TRANSFER_SRC,
                    .sharing = SHARING_MODE::CONCURRENT,
                });
            }
            bufferManager->memoryCopy(std::min(size, length), data, stagingBuffer);
        }
        else{
            auto manager = std::shared_ptr(m_manager); 
            auto bufferManager = manager->getBufferManager();
            
            auto& resource = get_content(0);
            bufferManager->memoryCopy(std::min(size, length), data, resource.buffer);
        }
    }
    
    bool GPUBuffer::ReadyForUse(BufferGPUData &content)
    {
        if(!(content).dirty &
            m_descriptor.sharing == SHARING_MODE::CONCURRENT)
            return true;
        return (content).dirty;
    }

    BufferGPUData GPUBuffer::CreateGPUData()
    {     
        auto manager = std::shared_ptr(m_manager);
        auto data = BufferGPUData{.buffer =  manager->getBufferManager()->reserveBuffer({
                            .request = std::max(size, 64u),
                            .usage = usage,
                            .sharing = m_descriptor.sharing,}),
                            .buffer_capacity = size};
        BufferReservation reserv;
        manager->getBufferManager()->getAddressReservation(data.buffer, reserv);
        return data;
    }

    void GPUBuffer::WriteTransfer(BufferGPUData &data, CommandBufferWriter<VkCommandBufferWriter> &writer) {
        if(m_descriptor.sharing == SHARING_MODE::EXCLUSIVE){
            auto manager = std::shared_ptr(m_manager)->getBufferManager();
            data.dirty = false;
            //std::cout << "copying to exclusive buffer" << std::endl;
            manager->queueCopy<VkCommandBufferWriter>(writer, stagingBuffer, data.buffer);
        }
    };

    void GPUBuffer::ReleaseData(BufferGPUData &data) {
        auto manager = std::shared_ptr(m_manager);
        auto bufferManager = manager->getBufferManager();
        bufferManager->freeBufferReservation(data.buffer);
    };

    void GPUBuffer::Release() {
        auto manager = std::shared_ptr(m_manager);
        auto bufferManager = manager->getBufferManager();

        if(stagingBuffer)
            bufferManager->freeBufferReservation(stagingBuffer);
    };
};
