#include "GPUBuffer.hpp"
#include "../modules/GPUResourceManager.hpp"
namespace boitatah{

    void GPUBuffer::copyData(void *data, uint32_t frameIndex)
    {
        
        dirty = true;
        //Stages a transfer
        if(descriptor.sharing == SHARING_MODE::EXCLUSIVE){
            auto manager = std::shared_ptr(m_manager); 
            auto bufferManager = manager->getBufferManager();


            if(!stagingBuffer.isNull())
                stagingBuffer = bufferManager->reserveBuffer({
                    .request = size,
                    .usage = usage,
                    .sharing = SHARING_MODE::CONCURRENT,
                });

            bufferManager->memoryCopy(size, data, stagingBuffer);
        }
        else{
            auto manager = std::shared_ptr(m_manager); 
            auto bufferManager = manager->getBufferManager();
            //auto& resource = self().self();
            auto& resource = get_content(frameIndex);
            auto frame_buffer = resource; //(self().get_content(frameIndex));
            // bufferManager->memoryCopy(size, data, frame_buffer.buffer);
        }
    }
    bool GPUBuffer::ReadyForUse(ResourceGPUContent<GPUBuffer> &content)
    {
                        if(descriptor.sharing == SHARING_MODE::CONCURRENT)
                    return true;
                return true;
                //return static_cast<MutableGPUResource<GPUBuffer>>(content).dirty;
    }
    void GPUBuffer::SetContent(ResourceGPUContent<GPUBuffer> &content)
    {
    }
    
    BufferGPUData GPUBuffer::CreateGPUData()
    {     
        auto manager = std::shared_ptr(m_manager); 
        return BufferGPUData{.buffer =  manager->getBufferManager()->reserveBuffer({
                            .request = size,
                            .usage = usage,
                            .sharing = SHARING_MODE::EXCLUSIVE,}),
                            .buffer_capacity = this->size};;
    }

    void GPUBuffer::WriteTransfer(BufferGPUData &data, CommandBufferWriter<VkCommandBufferWriter> &writer) {
        
        if(descriptor.sharing == SHARING_MODE::EXCLUSIVE){
            auto manager = std::shared_ptr(m_manager);
            auto bufferManager = manager->getBufferManager();
            data.dirty = false;
            
            bufferManager->queueCopy(stagingBuffer, data.buffer);
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

        bufferManager->freeBufferReservation(stagingBuffer);

    
    };
};
