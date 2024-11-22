#include "Buffer.hpp"
#include "BufferManager.hpp"
#include "../vulkan/Vulkan.hpp"
#include <bit>
namespace boitatah::buffer
{


    Buffer::Buffer(const BufferDesc &desc, const vk::Vulkan *vulkan) : vulkan(vulkan)
    {

        description = desc;

        // get the vulkan buffer
        // allocate the device memory
        auto reqs = this->vulkan->getBufferAlignmentMemoryType({
            .size = desc.estimatedElementSize,
            .usage = desc.usage,
            .sharing = desc.sharing,
        });

        alignment = reqs.alignment;
        sharing = desc.sharing;
        usage = desc.usage;
        buffer_id = buffer_quantity;
        
        if(sharing == SHARING_MODE::EXCLUSIVE)
            bufferManager = desc.bufferManager;

        setupBuffer(desc.estimatedElementSize, desc.partitions);

        blockSize = mainAllocator->getPartitionSize();
        buffer_quantity += 1;

    }

    Buffer::~Buffer(void)
    {
        vulkan->destroyBuffer({.buffer = bufferData.buffer, .memory = bufferData.memory});

        if(sharing == SHARING_MODE::CONCURRENT)
            vulkan->unmapMemory({bufferData.memory});

        buffer_quantity -= 1;
    }

    uint32_t Buffer::getID() const
    {
        return buffer_id;
    }

    bool Buffer::getReservationData(const Handle<BufferReservation> handle, BufferReservation &reservation) const
    {
        if(mainReservPool->get(handle, reservation))
            return true;
        return false;
    }

    Handle<BufferReservation> Buffer::reserve(const uint32_t request)
    {
        auto blockHandle = mainAllocator->allocate(request);

        // failed to reserve a buffer.
        if (blockHandle.isNull())
            return Handle<BufferReservation>{};

        BufferReservation reservation{};
        mainAllocator->getBlockData(blockHandle, reservation.offset, reservation.size);

        reservation.reservedBlock = blockHandle;
        reservation.requestSize = request;

        Handle<BufferReservation> reservationHandle = mainReservPool->set(reservation);
        return reservationHandle;
    }


    bool Buffer::unreserve(const Handle<BufferReservation> reservation)
    {
        if (reservation.isNull())
            return false;

        BufferReservation bufferReservation;
        if(!mainReservPool->get(reservation, bufferReservation))
            throw std::runtime_error("reservation double release");
        
        mainAllocator->release(bufferReservation.reservedBlock);

        return true;
    }

    void Buffer::copyData(const Handle<BufferReservation> handle, void *data)
    {
        BufferReservation reservation;
        if(!mainReservPool->get(handle, reservation)) 
            throw std::runtime_error("failed to copy data to gpu buffer");

        vulkan->copyToMappedMemory({
            .offset = reservation.offset,
            .elementSize = reservation.size,
            .elementCount = static_cast<uint32_t>(1),
            .map = mappedMemory,
            .data = data,
        });
    }

    //TODO IMPLEMENT
    void Buffer::queueUpdates()
    {
        std::shared_ptr<BufferManager> manager(bufferManager);
        //do the queues

        for(auto& transfer : queuedTransfers)
        {
            Buffer* srcBuffer;
            Buffer* dstBuffer;

            BufferReservation srcReservation;
            BufferReservation dstReservation;

            if(!manager->getAddressBuffer(transfer.stagingBufferAddress,srcBuffer))
                std::runtime_error("buffer transfer failed");
            
            if(!manager->getAddressBuffer(transfer.finalBufferAddress,dstBuffer))
                std::runtime_error("buffer transfer failed");

            if(!manager->getAddressReservation(transfer.stagingBufferAddress,srcReservation))
                std::runtime_error("buffer transfer failed");
            
            if(!manager->getAddressReservation(transfer.finalBufferAddress,dstReservation))
                std::runtime_error("buffer transfer failed");

            vulkan->CmdCopyBuffer({
                .commandBuffer = manager->getTransferBuffer().buffer,
                .srcBuffer = srcBuffer->getBuffer(),
                .srcOffset = srcReservation.offset,
                .dstBuffer = dstBuffer->getBuffer(),
                .dstOffset = dstReservation.offset,
                .size = dstReservation.requestSize,
            });
        }

    }

    void Buffer::clearTransferQueue()
    {
        queuedTransfers.clear();
    }

    bool Buffer::checkCompatibility(const BufferReservationRequest &compatibility)
    {
        return usage == compatibility.usage && 
               sharing == compatibility.sharing &&
               mainAllocator->getLargestFreeBlockSize() >= compatibility.request;
    }

    VkBuffer Buffer::getBuffer() const
    {
            return bufferData.buffer;
    }
    VkDeviceMemory Buffer::getMemory() const
    {
        return bufferData.memory;
    }

    void Buffer::setupBuffer(uint32_t desiredSize, uint32_t partitions){

        mainAllocator.reset(new BufferAllocator({.alignment = alignment,
                                        .partitionSize = desiredSize,
                                        .height = static_cast<uint32_t>(std::bit_width(partitions)) - 1u,
                                        }));

        bufferData = this->vulkan->createBuffer({
            .size = mainAllocator->getSize(),
            .usage = usage,
            .sharing = sharing,
        });
        
        mainReservPool.reset(new Pool<BufferReservation>(
            {.size = partitions, .name = "buffer pool"}));
    


    }

}

