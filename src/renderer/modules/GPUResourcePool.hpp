#ifndef BOITATAH_RESOURCE_POOL_HPP
#define BOITATAH_RESOURCE_POOL_HPP

#include <memory>
#include "../../collections/Pool.hpp"
#include "../../types/GPUBuffer.hpp"
#include "../../types/GeometryTEMP.hpp"


namespace boitatah{


    class GPUResourcePool{

        public:

            GPUResourcePool(){
                
                m_geometryPool = std::make_unique<Pool<Geometry>>(PoolOptions{
                    .size = 1024,
                    .dynamic = true,
                    .name = "geometry pool",
                });

                m_gpuBufferPool = std::make_unique<Pool<GPUBuffer>>(PoolOptions{
                    .size = 1024,
                    .dynamic = true,
                    .name = "gpu buffer pool",
                });


            };

            template<typename T> 
            T& get(Handle<T> handle);
            
            template<typename T> 
            Handle<T>  set(T& item);
            
            template<typename T> 
            bool update(Handle<T> handle, T& item);

            template<typename T> 
            bool clear(Handle<T> handle, T& item);

        private:
            std::unique_ptr<Pool<Geometry>> m_geometryPool;
            std::unique_ptr<Pool<GPUBuffer>> m_gpuBufferPool;    


    };

    template <>
    inline  GPUBuffer& GPUResourcePool::get(Handle<GPUBuffer> handle)
    {
        return m_gpuBufferPool->get(handle);
    }

    template<>
    inline  Handle<GPUBuffer>  GPUResourcePool::set(GPUBuffer& item)
    {
        return m_gpuBufferPool->set(item);
    }

}

#endif