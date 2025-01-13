#pragma once

#include <memory>
#include <collections/Pool.hpp>


namespace boitatah{

    class GPUBuffer;
    class Geometry;
    //class Geometry;
    class GPUResourcePool{

        public:

            GPUResourcePool();

            Geometry& get(Handle<Geometry> handle);
            Handle<Geometry>  set(Geometry& item);
            bool update(Handle<Geometry> handle, Geometry& item); 
            bool clear(Handle<Geometry> handle, Geometry& item);
            bool clear(Handle<Geometry> handle);

            GPUBuffer& get(Handle<GPUBuffer> handle);
            Handle<GPUBuffer>  set(GPUBuffer& item);
            bool update(Handle<GPUBuffer> handle, GPUBuffer& item); 
            bool clear(Handle<GPUBuffer> handle, GPUBuffer& item);
            bool clear(Handle<GPUBuffer> handle);


        private:
            std::unique_ptr<Pool<Geometry>> m_geometryPool;
            std::unique_ptr<Pool<GPUBuffer>> m_gpuBufferPool;

    };

};

