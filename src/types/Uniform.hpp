#ifndef  BOITATAH_UNIFORM_HPP 
#define BOITATAH_UNIFORM_HPP

#include "../collections/Pool.hpp"
#include "BttEnums.hpp"
#include "../buffers/Buffer.hpp"

namespace boitata{
    struct UniformCreateDesc{

    };

    // Needs a material level setter
    struct Uniform // a uniform buffer object
    {

    private:
        // GPU data
        Handle<BufferReservation> buffers[2]; // one for writing and one for reading
        DESCRIPTOR_TYPE type;

        void *data; // most current data version. align on 32/64
        uint32_t size;
        uint8_t reading = 0; // one or zero
        uint8_t dirty;
        // TODO
        //uint8_t read_only = 0; // only needs one buffer.

    public:

        // TODO fix the fact that i need this for vector resize.
        Uniform(){};
        Uniform(UniformCreateDesc &desc)
        {

        };

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
        Handle<BufferReservation> get_buffer(uint32_t frameIndex) const
        {
            return buffers[frameIndex % 2];
        };
        const void* get_data() const {return static_cast<const void*>(data);};
        uint32_t get_size() const {return size;};

    };
}
#endif //BOITATAH_UNIFORM_HPP