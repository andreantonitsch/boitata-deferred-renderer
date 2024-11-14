#include "BufferManager.hpp"
#include <algorithm>


namespace boitatah::buffer
{

    Handle<Buffer *> BufferManager::createBuffer(const BufferDesc &&description)
    {
        Buffer * buffer = new Buffer(description, m_vk);
        Handle<Buffer *> bufferHandle = bufferPool.set(buffer);
        activeBuffers.push_back(bufferHandle);
        return bufferHandle;
    }

    void BufferManager::releaseBuffer(Handle<Buffer *> handle)
    {
        // get buffer
        Buffer* buffer;
        if(!bufferPool.get(handle, buffer)){
            std::runtime_error("doubly_released buffer");
       }
        // delete buffer from pool of buffers
        bufferPool.clear(handle, buffer);
        
        // delete it from buffer list of active buffers
        auto position = std::find(activeBuffers.begin(), activeBuffers.end(), handle);
        if(position != activeBuffers.end()){
            activeBuffers.erase(position);
        }

        //delete buffer
        delete buffer;
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
                .partitions = 1 << 10,
                .usage = compatibility.usage,
                .sharing = compatibility.sharing,
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
                bufferPool.get(activeBuffers[i], buffer);
                if (buffer->checkCompatibility(compatibility))
                    return i;
            }
            return UINT32_MAX;
        }
    }

    BufferManager::BufferManager(Vulkan *vk_instance)
    {
        m_vk = vk_instance;
        m_trasnferFence = m_vk->createFence({});
        transferBuffer.buffer = m_vk->allocateCommandBuffer({.count = 1, 
                        .level = COMMAND_BUFFER_LEVEL::PRIMARY,
                        .type = COMMAND_BUFFER_TYPE::TRANSFER });
        transferBuffer.type = COMMAND_BUFFER_TYPE::TRANSFER;
    }

    BufferManager::~BufferManager()
    {
        m_vk->destroyFence(m_trasnferFence);
    }


    Handle<BufferAddress> BufferManager::reserveBuffer(const BufferReservationRequest &request)
    {

        Handle<Buffer *> bufferHandle = findOrCreateCompatibleBuffer(request);
        if(bufferHandle.isNull())
            return Handle<BufferAddress>();

        Buffer * buffer;
        bufferPool.get(bufferHandle, buffer);

        BufferAddress bufferAddress{
            .buffer = bufferHandle,
            .reservation = buffer->reserve(request.request),
            .size = static_cast<uint32_t>(request.request)
        };

        return addressPool.set(bufferAddress);
    }
    // TODO
    Handle<BufferAddress> BufferManager::uploadBuffer(const BufferUploadDesc &desc)
    {
        return Handle<BufferAddress>();
    }
    
    // TODO
    void BufferManager::queueUpdateReservation(Handle<BufferAddress> reservation, void *new_data)
    {
        //BufferReservation reservation;

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
                .fence = m_trasnferFence
            }
        );
    }

    const BufferReservation &BufferManager::getAddressReservation(const Handle<BufferAddress> handle) const
    {
        // TODO: insert return statement here
    }

    Buffer *BufferManager::getAddressBuffer(const Handle<BufferAddress> handle)
    {
        return nullptr;
    }

    void BufferManager::freeBufferReservation(Handle<BufferAddress> handle)
    {
    }

    bool BufferManager::areTransfersFinished() const
    {
        return m_vk->checkFenceStatus(m_trasnferFence);
    }
    void BufferManager::waitForTransferToFinish() const
    {
        return m_vk->waitForFence(m_trasnferFence);
    }
}
