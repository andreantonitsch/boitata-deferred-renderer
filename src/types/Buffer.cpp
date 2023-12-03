#include "Buffer.hpp"
#include "../vulkan/Vulkan.hpp"
#include <bit>
namespace boitatah
{

    Buffer::Buffer(const BufferDesc &desc, const vk::Vulkan *vulkan) : vulkan(vulkan)
    {

        this->description = desc;

        // get the vulkan buffer
        // allocate the device memory
        auto objs = this->vulkan->createBuffer({
            .size = desc.size,
            .usage = desc.usage,
            .sharing = desc.sharing,
        });
        buffer = objs.buffer;
        memory = objs.memory;
        actualSize = objs.actualSize;
        alignment = objs.alignment;

        // initialize the allocator
        allocator = new BufferAllocator({.alignment = objs.alignment,
                                         .partitionSize = actualSize / desc.partitions,
                                         .height = static_cast<uint32_t>(std::bit_width(actualSize / desc.partitions)) - 1u});
    }

    Buffer::~Buffer(void)
    {
        vulkan->destroyBuffer({.buffer = buffer, .memory = memory});
        delete allocator;
    }
    BufferReservation Buffer::reserve(const uint32_t request)
    {

        auto blockHandle = allocator->allocate(request);

        // failed to reserve a buffer.
        if (blockHandle.isNull())
            return BufferReservation{};

        Block block;
        BufferReservation reservation{
            .buffer = this,
        };
        allocator->getBlockData(blockHandle, reservation.offset, reservation.size);

        reservation.reservedBlock = blockHandle;

        return reservation;
    }
    bool Buffer::checkCompatibility(const BufferCompatibility &compatibility)
    {
        return usage == compatibility.usage && 
        allocator->freeSpace() > compatibility.requestSize;
    }
    VkBuffer Buffer::getBuffer()
    {
        return buffer;
        ;
    }
}
