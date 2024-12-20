#ifndef BOITATAH_GPURESOURCE_TEMP_HPP
#define BOITATAH_GPURESOURCE_TEMP_HPP

#include "../collections/Pool.hpp"
#include "../buffers/BufferStructs.hpp"
#include "../buffers/Buffer.hpp"
#include "BttEnums.hpp"
#include <stdexcept>

namespace boitatah{
    using namespace boitatah::buffer;
    class GPUResourceManager;

    enum class RESOURCE_TYPE{
        GPU_BUFFER,
        GEOMETRY,
    };

    enum class RESOURCE_MUTABILITY{
        IMMUTABLE,
        MUTABLE
    };

    struct ResourceDescriptor{
        SHARING_MODE sharing;
        RESOURCE_TYPE type;
        RESOURCE_MUTABILITY mutability;
    };



    template<typename T>
    struct ResourceUpdateDescription{

    };

    template<typename T>
    struct ResourceCreateDescription{

    };

    /// @brief Resource content to be shipped to gpu buffers
    /// @tparam type of resource 
    template<typename T>
    struct ResourceGPUContent{ };

    /// @brief Meta Resources this resource needs or points to,
    ///        p.e. Handles to other resources and//or buffers.
    ///        Not shipped to GPU buffers
    /// @tparam Final type of resource
    template<typename T>
    struct ResourceMetaContent{ };

    using namespace boitatah::buffer;
    template<typename DerivedResource>
    struct GPUResource // gpu data + metadata object
    {
        friend GPUResourceManager;
        protected:
            GPUResource( ) = default;

            ResourceDescriptor descriptor;
            std::weak_ptr<GPUResourceManager> m_manager;

            uint8_t dirty = 255u;
            bool commited = 255u;

            DerivedResource& self(){return *static_cast<DerivedResource*>(this);};

            void set_descriptor(const ResourceDescriptor &descriptor){
                this->descriptor = descriptor;
            }

            void set_content(uint32_t frameIndex, Handle<ResourceContent<DerivedResource>> content_data){
                self().__imp_set_content(frameIndex, content);
            }


            uint32_t get_size() const {return self().__impl_get_size();};//descriptor.size;};

            void clean_dirt(int frameIndex = 0) { dirty = dirty & ~(static_cast<uint8_t>(1u) << (frameIndex%2)); };

            void clean_commit(int frameIndex = 0) { commited = 0u;};

            void write(int frameIndex = 0){};

        public:
            ResourceContent<DerivedResource> get_content(uint32_t frameIndex) const
            {
                return self().__impl_resource_get_content(frameIndex);
            };

            void check_content_ready(int frameIndex){
                return self().__impl_check_content_ready(content[frameIndex%2]);
            };
            bool ready_for_use(uint32_t frameIndex) { return self().__impl_ready_for_use(frameIndex)};

            bool check_dirt(uint32_t frameIndex) { return static_cast<uint8_t>(0) < (dirty & (static_cast<uint8_t>(1) << (frameIndex%2))); };
        
            bool check_commited(uint32_t frameIndex) { return static_cast<uint8_t>(0u) == (dirty & (static_cast<commited>(1u) << (frameIndex%2))); };
        
            void update(const ResourceUpdateDescription<DerivedResource> &updateDescription){
                if(descriptor.mutability == RESOURCE_MUTABILITY::IMMUTABLE)
                    throw runtime_error("Immutable resource update attempt");
                
                self().__impl_resource_update(updateDescription);
                set_dirty();
            };

            void set_dirty()
            {
                dirty = 255u;
            };

            void set_commited(uint32_t frame_index)
            {
                 commited = commited & ~(static_cast<uint8_t>(1u) << (frameIndex%2));
            };

            /// @brief commits to update this resource next time resources are updated
            void commit()
            {
                commited = true;
                self.__impl_commit();
            };

            uint32_t get_data(void* const dstPtr, uint32_t frame_index) const {
                if(commited)
                    return self().__impl_get_data(dstPtr, frame_index);

                if(check_dirt(0) || check_dirt(1)))
                    commit();

                return self().__impl_get_data(dstPtr, frame_index);
                //static_cast<const void*>(resource_data.data);
            };

        };

    using namespace boitatah::buffer;
    template<typename DerivedResource>
    struct MutableGPUResource : GPUResource<MutableGPUResource<DerivedResource>>// gpu data + metadata object
    {
        private :
            ResourceGPUContent<DerivedResource> gpu_content[2];

        public :
            void __impl_resource_update(ResourceUpdateDescription<DerivedResource>& description){{

            }};

            ResourceGPUContent& __impl_resource_get_content(uint32_t frame_index){
                return content[frame_index % 2];
            };

            uint32_t __impl_get_data(void* dstPtr, frame_index){

            };

            void __impl_set_content(uint32_t frameIndex, ResourceGPUContent<DerivedResource> &content_data){
                gpu_content[frame_index %2 ] = self().__imp_set_content(content);
            };

            bool __impl_ready_for_use(uint32_t frame_index){
                return check_dirt(frame_index) ;
            };

    };

    using namespace boitatah::buffer;
    template<typename DerivedResource>
    struct ImmutableGPUResource : GPUResource<ImmutableGPUResource<DerivedResource>>// gpu data + metadata object
    {         
        private :   
            ResourceGPUContent<DerivedResource> gpu_content;
        public :

            void __impl_resource_update(ResourceUpdateDescription<DerivedResource>& description){{

            }};

            ResourceGPUContent& __impl_resource_get_content(uint32_t frame_index){
                return content;
            };

            uint32_t __impl_get_data(void* dstPtr, frame_index){

            };

            void __impl_set_content(uint32_t frameIndex, Handle<ResourceContent<DerivedResource>> content_data){
                gpu_content = self().__impl_set_content(content);
            };

            void get_data(){};
    };

}

#endif