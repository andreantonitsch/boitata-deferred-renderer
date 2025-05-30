#include <boitatah/buffers/BufferManager.hpp>
#include <boitatah/buffers/Buffer.hpp>
#include <boitatah/buffers/BufferStructs.hpp>
#include <algorithm>
#include <boitatah/backend/vulkan/Vulkan.hpp>
#include <boitatah/backend/vulkan/VkCommandBufferWriter.hpp>

namespace boitatah::buffer
{
    using Vulkan = boitatah::vk::VulkanInstance;
    using VkCommandBufferWriter = boitatah::vk::VkCommandBufferWriter;
    
    BufferManager::BufferManager(std::shared_ptr<vk::VulkanInstance> vk_instance)
    {
        m_vk = vk_instance;
        m_transferFence = m_vk->create_fence(true);


        m_transferBuffer = m_vk->allocate_commandbuffer({.count = 1, 
                        .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                        .type = COMMAND_BUFFER_TYPE::TRANSFER });
    }

    BufferManager::~BufferManager(void)
    {

        if(m_vk->check_fence_status(m_transferFence))
            m_vk->destroy_fence(m_transferFence);
        else{
            m_vk->wait_for_fence(m_transferFence);
            m_vk->destroy_fence(m_transferFence);
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

            std::runtime_error("reserve buffer failed. no buffer created.");
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
        auto handle = m_addressPool.set(bufferAddress);

        BufferReservation reserv;
        getAddressReservation(handle, reserv);
        return handle;
    }

    BufferAccessData BufferManager::getBufferAccessData(const Handle<BufferAddress> &handle)
    {
        BufferAccessData data;
        getAddressBuffer(handle, data.buffer);
        BufferReservation reserv;
        data.buffer->getReservationData(getAddressReservation(handle), reserv);
        data.offset = reserv.offset;
        data.size = reserv.size;

        return data;
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
    
    void BufferManager::memoryCopy(uint32_t dataSize, const void *data, Handle<BufferAddress> &handle)
    {
        //TODO handle except
        auto& bufferAddr = m_addressPool.get(handle);
        auto& buffer = m_bufferPool.get(bufferAddr.buffer);
        
        BufferReservation reserv;
        if(!buffer->getReservationData(bufferAddr.reservation, reserv));
            std::runtime_error("failed to get reservation data in buffer manager memory copy");
        if(reserv.size < dataSize)
            std::runtime_error("Buffer is smaller than required space in buffermanager copy.");

        buffer->copyData(bufferAddr.reservation, data, dataSize);
        
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
        std::cout << "failed get address buffer on buffer " << handle.i << " generation " << handle.gen << std::endl;
        return false;
    }

    void BufferManager::freeBufferReservation(Handle<BufferAddress> handle)
    {
    }

    bool BufferManager::areTransfersFinished() const
    {
        return m_vk->check_fence_status(m_transferFence);
    }
    void BufferManager::waitForTransferToFinish() const
    {
        return m_vk->wait_for_fence(m_transferFence);
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
        BufferReservation srcReservation;
        BufferReservation dstReservation;
        if(getAddressBuffer(dst, dstBuffer) &&
           getAddressBuffer(src, srcBuffer) &&
           getAddressReservation(dst, dstReservation) &&
           getAddressReservation(src, srcReservation)){

            writer.copy_buffer({
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