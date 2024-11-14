#include "Buffer.hpp"
#include "../vulkan/Vulkan.hpp"
#include <bit>
namespace boitatah::buffer
{

    Buffer::Buffer(const BufferDesc &desc, const vk::Vulkan *vulkan) : vulkan(vulkan)
    {

        this->description = desc;

        // get the vulkan buffer
        // allocate the device memory
        auto reqs = this->vulkan->getBufferAlignmentMemoryType({
            .size = desc.estimatedElementSize,
            .usage = desc.usage,
            .sharing = desc.sharing,
        });
        alignment = reqs.alignment;

        // initialize the allocator
        allocator = new BufferAllocator({.alignment = alignment,
                                         .partitionSize = desc.estimatedElementSize,
                                         .height = static_cast<uint32_t>(std::bit_width(desc.partitions)) - 1u,
                                         });

        auto objs = this->vulkan->createBuffer({
            .size = allocator->getSize(),
            .usage = desc.usage,
            .sharing = desc.sharing,
        });

        sharing = desc.sharing;
        usage = desc.usage;
        actualSize = objs.actualSize;
        buffer = objs.buffer;
        memory = objs.memory;
    
    }

    Buffer::~Buffer(void)
    {
        vulkan->destroyBuffer({.buffer = buffer, .memory = memory});
        delete allocator;
    }
    
    Handle<BufferReservation> Buffer::reserve(const uint32_t request)
    {
        auto blockHandle = allocator->allocate(request);

        // failed to reserve a buffer.
        if (blockHandle.isNull())
            return Handle<BufferReservation>{};

        BufferReservation reservation{};
        allocator->getBlockData(blockHandle, reservation.offset, reservation.size);

        reservation.reservedBlock = blockHandle;
        reservation.requestSize = request;

        Handle<BufferReservation> reservationHandle = reservationPool.set(reservation);
        return reservationHandle;
    }


    bool Buffer::unreserve(const Handle<BufferReservation> reservation)
    {
        if (reservation.isNull())
            return false;

        BufferReservation bufferReservation;
        if(!reservationPool.get(reservation, bufferReservation))
            throw std::runtime_error("reservation double release");
        
        allocator->release(bufferReservation.reservedBlock);

        return true;
    }

    // TODO implement
    void Buffer::queueUpdate(const Handle<BufferReservation> handle, void *data)
    {



    }

    bool Buffer::checkCompatibility(const BufferReservationRequest &compatibility)
    {
        return usage == compatibility.usage && 
               allocator->getLargestFreeBlockSize() >= compatibility.request;
    }
    VkBuffer Buffer::getBuffer()
    {
        return buffer;
    }
    VkDeviceMemory Buffer::getMemory()
    {
        return memory;
    }
}
