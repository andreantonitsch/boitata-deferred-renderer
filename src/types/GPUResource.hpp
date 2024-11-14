#ifndef BOITATAH_GPURESOURCE_HPP
#define BOITATAH_GPURESOURCE_HPP

#include "../collections/Pool.hpp"
#include "../buffers/BufferStructs.hpp"
#include "../buffers/Buffer.hpp"
#include "BttEnums.hpp"


namespace boitatah{

    using namespace boitatah::buffer;
    struct GPUResource // a uniform buffer object
    {

        private:
            // GPU data
            Handle<BufferAddress> buffers[2]; // one for writing and one for reading
            DESCRIPTOR_TYPE type;

            void *data; // most current data version. align on 32/64
            uint32_t size;
            uint8_t reading = 0; // one or zero
            uint8_t dirty;
            // TODO
            //uint8_t read_only = 0; // only needs one buffer.

        public:
            // new size has to be equal to old size.
            void change_data(void *new_data)
            {
                data = new_data;
                flag_update();
            };

            void flag_update()
            {
                dirty = 255u; 
            };

            void clean_frame_index(int index) { dirty = dirty & ~(static_cast<uint8_t>(1) << index); };
            bool check_dirt(int index) { return static_cast<uint8_t>(0) < (dirty & (static_cast<uint8_t>(1) << index)); };
            Handle<BufferAddress> get_buffer(uint32_t frameIndex) const
            {
                return buffers[frameIndex % 2];
            };
            const void* get_data() const {return static_cast<const void*>(data);};
             uint32_t get_size() const {return size;};

        };

}

#endif