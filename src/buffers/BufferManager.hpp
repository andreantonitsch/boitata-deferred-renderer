#ifndef BOITATAH_BUFFER_MANAGER_HPP
#define BOITATAH_BUFFER_MANAGER_HPP

#include "Renderer.hpp"
#include "../types/Buffer.hpp"
#include "../collections/Pool.hpp"

namespace boitatah
{
    class BufferManager
    {
        private:

        
        
        public:
            Handle<BufferReservation> reserveBuffer();

    };
};

#endif //BOITATAH_BUFFER_MANAGER_HPP