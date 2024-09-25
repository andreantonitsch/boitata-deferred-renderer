#ifndef BOITATAH_BUFFER_MANAGER_HPP
#define BOITATAH_BUFFER_MANAGER_HPP

#include <vector>
#include "Renderer.hpp"
#include "Buffer.hpp"
#include "../collections/Pool.hpp"

namespace boitatah
{
    class BufferManager
    {
        private:
            Vulkan* m_vk;
            std::vector<Handle<Buffer *>> activeBuffers;
            Pool<Buffer *> bufferPool;
            


            Handle<Buffer*> createBuffer(const BufferDesc &&description);
            void releaseBuffer();
            void updateBufferSync(void* new_data);

            void startBufferUpd();
            void endBufferUpd();

            Handle<Buffer *> findOrCreateCompatibleBuffer(const BufferCompatibility &compatibility);
            uint32_t findCompatibleBuffer(const BufferCompatibility &compatibility);
        
        public:
            Handle<BufferReservation> reserveBuffer();
            void updateReservation(Handle<BufferReservation> reservation, void* new_data); //updates synchronously
            void queueUpdateReservation(Handle<BufferReservation> reservation, void* new_data); //queues updates
            void startBufferUpdates(); //setup queue buffer updates
            void endBufferUpdates();    // ship queue buffer updates

            void freeBufferReservation(Handle<BufferReservation> reservation);

    };
};

#endif //BOITATAH_BUFFER_MANAGER_HPP