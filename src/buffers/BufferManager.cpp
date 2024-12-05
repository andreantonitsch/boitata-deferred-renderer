#include "BufferManager.hpp"
#include "Buffer.hpp"
#include "BufferStructs.hpp"
#include <algorithm>


namespace boitatah::buffer
{

    BufferManager::BufferManager(Vulkan *vk_instance)
    {
        m_vk = vk_instance;
        m_transferFence = m_vk->createFence(true);


        transferBuffer.buffer = m_vk->allocateCommandBuffer({.count = 1, 
                        .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                        .type = COMMAND_BUFFER_TYPE::TRANSFER });
        transferBuffer.type = COMMAND_BUFFER_TYPE::TRANSFER;
    }

    BufferManager::~BufferManager(void)
    {

        if(m_vk->checkFenceStatus(m_transferFence))
            m_vk->destroyFence(m_transferFence);
        else{
            m_vk->waitForFence(m_transferFence);
            m_vk->destroyFence(m_transferFence);
        }

        std::cout << "Cleared buffer manager fence" << std::endl;

        while(activeBuffers.size()){
            releaseBuffer(activeBuffers.back());
        }


    }

    Handle<Buffer *> BufferManager::createBuffer(const BufferDesc &&description)
    {
        Buffer * buffer = new Buffer(description, m_vk);
        Handle<Buffer *> bufferHandle = bufferPool.set(buffer);
        activeBuffers.push_back(bufferHandle);
        std::cout << "Created Buffer" << buffer->getID() <<std::endl;
        return bufferHandle;
    }

    void BufferManager::releaseBuffer(Handle<Buffer *> handle)
    {
        // get buffer
        Buffer* buffer;
        if(!bufferPool.tryGet(handle, buffer)){
            std::runtime_error("doubly released buffer");
       }
        // delete buffer from pool of buffers
        bufferPool.clear(handle, buffer);
        
        // delete it from buffer list of active buffers
        auto position = std::find(activeBuffers.begin(), activeBuffers.end(), handle);
        if(position != activeBuffers.end()){
            activeBuffers.erase(position);
        }
        else{
            std::cout << "failed to find buffer in active buffers" << std::endl;
        }
        

        //delete buffer
        delete buffer;
        std::cout << "Deleted buffer " << buffer->getID() << std::endl;
    }

    Handle<Buffer *> BufferManager::findOrCreateCompatibleBuffer(const BufferReservationRequest &compatibility)
    {
        // Find buffer
        uint32_t bufferIndex = findCompatibleBuffer(compatibility);
        if (bufferIndex != UINT32_MAX)
        {
            // return handle copy
            return activeBuffers[bufferIndex];
        }
        else
        {
            Handle<Buffer *> buffer = createBuffer({
                .estimatedElementSize = compatibility.request,
                .partitions = partitionsPerBuffer,
                .usage = compatibility.usage,
                .sharing = compatibility.sharing,
                .bufferManager = shared_from_this()
                // .stagingBuffer = compatibility.sharing == SHARING_MODE::CONCURRENT ?
                //                  nullptr :
                //                  findOrCreateCompatibleStagingBuffer(compatibility)
            });
            // return handle copy
            return buffer;
        }
    }

    uint32_t BufferManager::findCompatibleBuffer(const BufferReservationRequest &compatibility)
    {
        {
            for (int i = 0; i < activeBuffers.size(); i++)
            {
                Buffer * buffer;
                bufferPool.tryGet(activeBuffers[i], buffer);
                if (buffer->checkCompatibility(compatibility))
                    return i;
            }            
            return UINT32_MAX;
        }
    }


    Handle<BufferAddress> BufferManager::reserveBuffer(const BufferReservationRequest &request)
    {

        Handle<Buffer *> bufferHandle = findOrCreateCompatibleBuffer(request);
        if(bufferHandle.isNull()){
            std::cout << "reserve buffer failed. no buffer created." << std::endl;
            return Handle<BufferAddress>();
        }

        Buffer * buffer;
        bufferPool.tryGet(bufferHandle, buffer);

        BufferAddress bufferAddress{
            .buffer = bufferHandle,
            .reservation = buffer->reserve(request.request),
            .size = static_cast<uint32_t>(request.request)
        };
            
        return addressPool.set(bufferAddress);
    }

    bool BufferManager::copyToBuffer(const BufferUploadDesc &desc)
    {
        std::cout << "starting copy to buffer" << std::endl;
        BufferAddress& address = addressPool.get(desc.address);

        Buffer*& buffer = bufferPool.get(address.buffer);
        
        buffer->copyData(address.reservation, desc.data);
        std::cout << "finished copy to buffer" << std::endl;
        return true;
    }
    
    void BufferManager::queueingBufferUpdates()
    {
        //std::cout << "queue buffer updates" << std::endl;
        for(auto& handle : activeBuffers){
            Buffer*& buffer = bufferPool.get(handle);
            buffer->queueUpdates();
        }
        //std::cout << "finished queue buffer updates" << std::endl;
    }

    void BufferManager::startBufferUpdates()
    {
        m_vk->resetCmdBuffer(transferBuffer.buffer);
        m_vk->beginCmdBuffer({.commandBuffer = transferBuffer.buffer});
    }

    void BufferManager::endBufferUpdates()
    {
        m_vk->submitCmdBuffer(
            {
                .commandBuffer = transferBuffer.buffer,
                .submitType = COMMAND_BUFFER_TYPE::TRANSFER,
                .fence = m_transferFence
            }
        );
    }

    bool BufferManager::getAddressReservation(const Handle<BufferAddress> handle, BufferReservation& reservation)
    {
        BufferAddress address;

        if(addressPool.tryGet(handle, address)){
            Buffer *buffer;
            if(bufferPool.tryGet(address.buffer, buffer)){
                buffer->getReservationData(address.reservation, reservation);
                return true;
            }
        }
        return false;
    }

    bool BufferManager::getAddressBuffer(const Handle<BufferAddress> handle, Buffer*& buffer)
    {
        BufferAddress address;
        if(addressPool.tryGet(handle, address)){
            if(bufferPool.tryGet(address.buffer, buffer))
            return true;
        }
        return false;
    }

    void BufferManager::freeBufferReservation(Handle<BufferAddress> handle)
    {
    }

    bool BufferManager::areTransfersFinished() const
    {
        return m_vk->checkFenceStatus(m_transferFence);
    }
    void BufferManager::waitForTransferToFinish() const
    {
        return m_vk->waitForFence(m_transferFence);
    }

    CommandBuffer BufferManager::getTransferBuffer()
    {
        return transferBuffer;
    }
}
