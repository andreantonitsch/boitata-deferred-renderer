#include "GPUResourcePool.hpp"

namespace boitatah{

    GPUResourcePool::GPUResourcePool()
    {
        
        
        // m_geometryPool = std::make_unique<Pool<Geometry>>(PoolOptions{
        //     .size = 1024,
        //     .dynamic = true,
        //     .name = "geometry pool",
        // });

        m_gpuBufferPool = std::make_unique<Pool<GPUBuffer*>>(PoolOptions{
            .size = 1024,
            .dynamic = true,
            .name = "gpu buffer pool",
        });
    }

    GPUBuffer & GPUResourcePool::get(Handle<GPUBuffer*> handle)
    {
        m_gpuBufferPool->get(handle);
    }

    Handle<GPUBuffer*> GPUResourcePool::set(GPUBuffer &item)
    {
        
        //return m_gpuBufferPool->set(item);
    }

};

