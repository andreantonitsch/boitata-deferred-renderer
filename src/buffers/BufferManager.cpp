#include "BufferManager.hpp"
#include "Buffer.hpp"
#include "BufferStructs.hpp"
#include <algorithm>


namespace boitatah::buffer
{

    BufferManager::BufferManager(std::shared_ptr<vk::Vulkan> vk_instance)
    {
        m_vk = vk_instance;
        m_transferFence = m_vk->createFence(true);


        m_transferBuffer.buffer = m_vk->allocateCommandBuffer({.count = 1, 
                        .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                        .type = COMMAND_BUFFER_TYPE::TRANSFER });
        m_transferBuffer.type = COMMAND_BUFFER_TYPE::TRANSFER;
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

        while(m_activeBuffers.size()){
            releaseBuffer(m_activeBuffers.back());
        }
        std::cout << "Cleared active buffers in Buffer Manager" << std::endl;
    }

    Handle<Buffer *> BufferManager::createBuffer(const BufferDesc &&description)
    {
        Buffer * buffer = new Buffer(description, m_vk.get());
        Handle<Buffer *> bufferHandle = m_bufferPool.set(buffer);
        m_activeBuffers.push_back(bufferHandle);
        std::cout << "Created Buffer" << buffer->getID() <<std::endl;
        return bufferHandle;
    }

    void BufferManager::releaseBuffer(Handle<Buffer *> handle)
    {
        // get buffer
        Buffer* buffer;
        if(!m_bufferPool.tryGet(handle, buffer)){
            std::runtime_error("doubly released buffer");
       }
        // delete buffer from pool of buffers
        m_bufferPool.clear(handle, buffer);
        
        // delete it from buffer list of active buffers
        auto position = std::find(m_activeBuffers.begin(), m_activeBuffers.end(), handle);
        if(position != m_activeBuffers.end()){
            m_activeBuffers.erase(position);
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
            return m_activeBuffers[bufferIndex];
        }
        else
        {
            Handle<Buffer *> buffer = createBuffer({
                .estimatedElementSize = compatibility.request,
                .partitions = partitionsPerBuffer,
                .usage = compatibility.usage,
                .sharing = compatibility.sharing,
                .bufferManager = shared_from_this(),
                });
            
            // return handle copy
            return buffer;
        }
    }

    uint32_t BufferManager::findCompatibleBuffer(const BufferReservationRequest &compatibility)
    {
        {
            for (int i = 0; i < m_activeBuffers.size(); i++)
            {
                Buffer * buffer;
                m_bufferPool.tryGet(m_activeBuffers[i], buffer);
                if (buffer->checkCompatibility(compatibility))
                    return i;
            }            
            return UINT32_MAX;
        }
    }


    Handle<BufferAddress> BufferManager::reserveBuffer(const BufferReservationRequest &request)
    {

        Handle<Buffer *> bufferHandle = findOrCreateCompatibleBuffer(request);
        if(!bufferHandle){
            std::cout << "reserve buffer failed. no buffer created." << std::endl;
            return Handle<BufferAddress>();
        }

        Buffer * buffer;
        if(!m_bufferPool.tryGet(bufferHandle, buffer)){
            std::runtime_error("failed to get buffer from buffer pool");
        }

        BufferAddress bufferAddress{
            .buffer = bufferHandle,
            .reservation = buffer->reserve(request.request),
            .size = static_cast<uint32_t>(request.request)
        };
            
        return m_addressPool.set(bufferAddress);
    }

    bool BufferManager::copyToBuffer(const BufferUploadDesc &desc)
    {
        std::cout << "starting copy to buffer" << std::endl;
        BufferAddress& address = m_addressPool.get(desc.address);

        Buffer*& buffer = m_bufferPool.get(address.buffer);
        
        buffer->copyData(address.reservation, desc.data);
        std::cout << "finished copy to buffer" << std::endl;
        return true;
    }

    bool BufferManager::queueCopy(const Handle<BufferAddress> src, const Handle<BufferAddress> dst)
    {
        Buffer* dstBuffer;
        if(getAddressBuffer(dst, dstBuffer)){

            auto dstReserv = getAddressReservation(dst);

            Buffer* srcBuffer;

            BufferReservation srcReservation;
            BufferReservation dstReservation;

            if(!getAddressBuffer(src, srcBuffer))
                std::runtime_error("buffer transfer failed, invalid staging buffer");
            
            if(!getAddressReservation(src,srcReservation))
                std::runtime_error("buffer transfer failed, invalid staging reservation");

            if(!dstBuffer->getReservationData(dstReserv,dstReservation))
                std::runtime_error("buffer transfer failed, invalid staging reservation");

            m_vk->CmdCopyBuffer({
                .commandBuffer = getTransferBuffer().buffer,
                .srcBuffer = srcBuffer->getBuffer(),
                .srcOffset = srcReservation.offset,
                .dstBuffer = dstBuffer->getBuffer(),
                .dstOffset = dstReservation.offset,
                .size = dstReservation.requestSize,
            });
            return true;
        }
        return false;
    }

    // void BufferManager::queueingBufferUpdates()
    // {
    //     //std::cout << "queue buffer updates" << std::endl;
    //     for(auto& handle : activeBuffers){
    //         Buffer*& buffer = bufferPool.get(handle);

    //         if(buffer != nullptr && buffer->hasUpdates())
    //             buffer->queueTransfers();
    //     }
    //     //std::cout << "finished queue buffer updates" << std::endl;
    // }

    void BufferManager::memoryCopy(uint32_t dataSize, void *data, Handle<BufferAddress> &handle)
    {
        //TODO handle except
        auto& bufferAddr = m_addressPool.get(handle);
        std::cout << "buffer address fetched" << std::endl;
        auto buffer = m_bufferPool.get(bufferAddr.buffer);
        std::cout << "buffer fetched" << std::endl;
        BufferReservation reserv;
        if(!buffer->getReservationData(bufferAddr.reservation, reserv));
            std::runtime_error("failed to get reservation data in buffer manager memory copy");
        if(reserv.size < dataSize)
            std::runtime_error("Buffer is smaller than required space in buffermanager copy.");

        buffer->copyData(bufferAddr.reservation, data);
        
    }

    void BufferManager::queueingBufferUpdates()
    {
        
    }

    void BufferManager::startBufferUpdates()
    {
        m_vk->resetCmdBuffer(m_transferBuffer.buffer);
        m_vk->beginCmdBuffer({.commandBuffer = m_transferBuffer.buffer});
    }

    void BufferManager::endBufferUpdates()
    {
        m_vk->submitCmdBuffer(
            {
                .commandBuffer = m_transferBuffer.buffer,
                .submitType = COMMAND_BUFFER_TYPE::TRANSFER,
                .fence = m_transferFence
            }
        );
    }

    bool BufferManager::getAddressReservation(const Handle<BufferAddress> handle, BufferReservation& reservation)
    {
        BufferAddress address;

        if(m_addressPool.tryGet(handle, address)){
            Buffer *buffer;
            if(m_bufferPool.tryGet(address.buffer, buffer)){
                buffer->getReservationData(address.reservation, reservation);
                return true;
            }
        }
        return false;
    }

    Handle<BufferReservation> BufferManager::getAddressReservation(const Handle<BufferAddress> handle)
    {
        BufferAddress address;
        if(m_addressPool.tryGet(handle, address)){
            Buffer *buffer;
            if(m_bufferPool.tryGet(address.buffer, buffer)){
                return address.reservation;
            }
        }
        return Handle<BufferReservation>();
    }

    bool BufferManager::getAddressBuffer(const Handle<BufferAddress> handle, Buffer*& buffer)
    {
        BufferAddress address;
        if(m_addressPool.tryGet(handle, address)){
            if(m_bufferPool.tryGet(address.buffer, buffer))
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
        return m_transferBuffer;
    }

    VkBuffer BufferManager::getVkBuffer(const Handle<BufferAddress> handle)
    {
        auto addr = m_addressPool.get(handle);

        auto buffer = m_bufferPool.get(addr.buffer);
        
        return buffer->getBuffer();
    }

    template <class T>
    bool BufferManager::queueCopy(CommandBufferWriter<T> &writer, const Handle<BufferAddress> src, const Handle<BufferAddress> dst)
    {
        Buffer* dstBuffer;
        Buffer* srcBuffer;
        if(getAddressBuffer(dst, dstBuffer) && getAddressBuffer(src, srcBuffer)){
 
            auto dstReservHandle = getAddressReservation(dst);
            auto srcReservHandle = getAddressReservation(src);

            BufferReservation srcReservation;
            BufferReservation dstReservation;

            if(dstBuffer->getReservationData(dstReservHandle, dstReservation))
                std::runtime_error("buffer transfer failed, invalid final reservation");

            if(srcBuffer->getReservationData(srcReservHandle, srcReservation))
                std::runtime_error("buffer transfer failed, invalid staging reservation");

            writer.copyBuffer({
                .srcBuffer = srcBuffer->getBuffer(),
                .srcOffset = srcReservation.offset,
                .dstBuffer = dstBuffer->getBuffer(),
                .dstOffset = dstReservation.offset,
                .size = dstReservation.requestSize,
            });
            
            return true;
        }
        return false;
    };
    template bool BufferManager::queueCopy(CommandBufferWriter<VkCommandBufferWriter> &writer, const Handle<BufferAddress> src, const Handle<BufferAddress> dst);

};