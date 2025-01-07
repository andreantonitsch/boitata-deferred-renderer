#include "GPUBuffer.hpp"
#include "../modules/GPUResourceManager.hpp"
namespace boitatah{

    void GPUBuffer::copyData(void *data)
    {
        
        dirty = true;
        //Stages a transfer
        if(m_descriptor.sharing == SHARING_MODE::EXCLUSIVE){
            std::cout << "GPUBuffer Exclusive Copy Data Invoked" << std::endl;
            auto manager = std::shared_ptr(m_manager); 
            auto bufferManager = manager->getBufferManager();


            if(!stagingBuffer){
                stagingBuffer = bufferManager->reserveBuffer({
                    .request = size,
                    .usage = usage,
                    .sharing = SHARING_MODE::CONCURRENT,
                });

                std::cout << "created staging buffer" << std::endl;
            }

            bufferManager->memoryCopy(size, data, stagingBuffer);
        }
        else{
            std::cout << "GPUBuffer Shared Copy Data Invoked" << std::endl;
            auto manager = std::shared_ptr(m_manager); 
            auto bufferManager = manager->getBufferManager();
            //auto& resource = self().self();
            auto& resource = get_content(0);
            auto frame_buffer = resource; //(self().get_content(frameIndex));
            //bufferManager->memoryCopy(size, data, frame_buffer.buffer);
        }
    }
    
    bool GPUBuffer::ReadyForUse(ResourceGPUContent<GPUBuffer> &content)
    {
                        if(m_descriptor.sharing == SHARING_MODE::CONCURRENT)
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
        auto data = BufferGPUData{.buffer =  manager->getBufferManager()->reserveBuffer({
                            .request = size,
                            .usage = usage,
                            .sharing = m_descriptor.sharing,}),
                            .buffer_capacity = size};

        std::cout << "Created GPUBuffer GPU Data with size " << size << " and usage "
                  <<  std::endl;

        return data;
    }

    void GPUBuffer::WriteTransfer(BufferGPUData &data, CommandBufferWriter<VkCommandBufferWriter> &writer) {
        
        if(m_descriptor.sharing == SHARING_MODE::EXCLUSIVE){
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
