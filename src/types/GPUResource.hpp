#ifndef BOITATAH_GPURESOURCE_HPP
#define BOITATAH_GPURESOURCE_HPP

#include "../collections/Pool.hpp"
#include "../buffers/BufferStructs.hpp"
#include "../buffers/Buffer.hpp"
#include "BttEnums.hpp"


namespace boitatah{

    struct ResourceDescriptor{
        SHARING_MODE sharing;
        BUFFER_USAGE usage;
        //RESOURCE_TYPE type;
        uint32_t size;
    };

    struct ResourceMetaData{
        uint32_t data_size;
        void* data;
    };

    struct BufferMetaData{
        Handle<BufferAddress> buffer;
        uint32_t buffer_capacity;
    };

    using namespace boitatah::buffer;
    struct GPUResource // gpu data + metadata object
    {

        private:
            // GPU data
            Handle<BufferAddress> buffers[2]; // one for writing and one for reading
            BufferMetaData buffers_meta_data[2];

            ResourceMetaData resource_data; // most current data version. align on 32/64
            ResourceDescriptor descriptor; //size;
            uint8_t reading = 0; // one or zero
            uint8_t dirty;


        public:
            void set_descriptor(ResourceDescriptor &descriptor){
                this->descriptor = descriptor;
            }

            void set_buffer(uint32_t frameIndex, Handle<BufferAddress> buffer){
                buffers[frameIndex % 2] = buffer;
            }

            // new size has to be equal to old size.
            void change_data(ResourceMetaData &new_data)
            {
                resource_data = new_data;
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

            const void* get_data() const {return static_cast<const void*>(resource_data.data);};
            uint32_t get_size() const {return descriptor.size;};
        
        };
}

#endif