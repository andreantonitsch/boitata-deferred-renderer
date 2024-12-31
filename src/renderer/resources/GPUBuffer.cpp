#include "GPUBuffer.hpp"
namespace boitatah{

    // auto BufferGPUData::getContentImp() const -> const BufferGPUData&
    // {
    //     return static_cast<const BufferGPUData&>(*this);
    // }


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
    bool GPUBuffer::__impl_ready_for_use(ResourceGPUContent<GPUBuffer> &content)
    {
                        if(descriptor.sharing == SHARING_MODE::CONCURRENT)
                    return true;
                return true;
                //return static_cast<MutableGPUResource<GPUBuffer>>(content).dirty;
    }
    BufferGPUData GPUBuffer::__impl_create_managed_resource()
    {     
        auto manager = std::shared_ptr(m_manager); 
        return BufferGPUData{.buffer =  manager->getBufferManager()->reserveBuffer({
                            .request = size,
                            .usage = usage,
                            .sharing = SHARING_MODE::EXCLUSIVE,}),
                            .buffer_capacity = this->size};;
    }
    void GPUBuffer::__impl_write_transfer(BufferGPUData &data, CommandBufferWriter<VkCommandBufferWriter> &writer) {
        
        if(descriptor.sharing == SHARING_MODE::EXCLUSIVE){
            auto manager = std::shared_ptr(m_manager);
            auto bufferManager = manager->getBufferManager();
            data.dirty = false;
            
            bufferManager->queueCopy(stagingBuffer, data.buffer);
    }
    };

};
