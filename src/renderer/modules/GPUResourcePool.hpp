#ifndef BOITATAH_RESOURCE_POOL_HPP
#define BOITATAH_RESOURCE_POOL_HPP

#include <memory>
#include "../../collections/Pool.hpp"


namespace boitatah{

    class GPUBuffer;
    //class Geometry;
    class GPUResourcePool{

        public:

            GPUResourcePool();

            GPUBuffer& get(Handle<GPUBuffer> handle);
            Handle<GPUBuffer>  set(GPUBuffer& item);
            bool update(Handle<GPUBuffer> handle, GPUBuffer& item); 
            bool clear(Handle<GPUBuffer> handle, GPUBuffer& item);
            bool clear(Handle<GPUBuffer> handle);


        private:
            //std::unique_ptr<Pool<Geometry>> m_geometryPool;
            std::unique_ptr<Pool<GPUBuffer>> m_gpuBufferPool;

    };

};

#endif