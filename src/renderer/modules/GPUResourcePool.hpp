#ifndef BOITATAH_RESOURCE_POOL_HPP
#define BOITATAH_RESOURCE_POOL_HPP

#include <memory>
#include "../../collections/Pool.hpp"

#include "../resources/GPUBuffer.hpp"

namespace boitatah{

    class GPUBuffer;
    //class Geometry;
    class GPUResourcePool{

        public:

            GPUResourcePool() = default;

            GPUBuffer& get(Handle<GPUBuffer*> handle);
            Handle<GPUBuffer*>  set(GPUBuffer& item);
            
            template<typename T> 
            bool update(Handle<T> handle, T& item);

            template<typename T> 
            bool clear(Handle<T> handle, T& item);

        private:
            //std::unique_ptr<Pool<Geometry>> m_geometryPool;
            std::unique_ptr<Pool<GPUBuffer*>> m_gpuBufferPool;    

    };

};

#endif