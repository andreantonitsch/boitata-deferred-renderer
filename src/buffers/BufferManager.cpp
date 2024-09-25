#include "BufferManager.hpp"

namespace boitatah
{
    Handle<Buffer *> BufferManager::createBuffer(const BufferDesc &&description)
    {
        Buffer * buffer = new Buffer(description, m_vk);
        Handle<Buffer *> bufferHandle = bufferPool.set(buffer);
        activeBuffers.push_back(bufferHandle);
        return bufferHandle;
    }

    Handle<Buffer *> boitatah::BufferManager::findOrCreateCompatibleBuffer(const BufferCompatibility &compatibility)
    {
        // Find buffer
        uint32_t bufferIndex = findCompatibleBuffer(compatibility);
        if (bufferIndex != UINT32_MAX)
        {
            // return reference
            return activeBuffers[bufferIndex];
        }
        else
        {
            Handle<Buffer *> buffer = createBuffer({
                .estimatedElementSize = compatibility.requestSize,
                .partitions = 1 << 10,
                .usage = compatibility.usage,
                .sharing = compatibility.sharing,
            });
            // return reference

            return buffer;
        }
    }

    uint32_t BufferManager::findCompatibleBuffer(const BufferCompatibility &compatibility)
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
