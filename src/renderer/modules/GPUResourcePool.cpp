#include "GPUResourcePool.hpp"
#include "../resources/GPUBuffer.hpp"
#include <types/Geometry.hpp>

namespace boitatah{
    
    GPUResourcePool::GPUResourcePool()
    {
        
        
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
    }

    Geometry& GPUResourcePool::get(Handle<Geometry> handle){
        return m_geometryPool->get(handle);
    }

    Handle<Geometry> GPUResourcePool::set(Geometry &item)
    {
        return m_geometryPool->set(item);
    }

    bool GPUResourcePool::clear(Handle<Geometry> handle, Geometry &item)
    {
        return m_geometryPool->clear(handle, item);
    }
    
    bool GPUResourcePool::clear(Handle<Geometry> handle)
    {
        return m_geometryPool->clear(handle);
    }


    GPUBuffer& GPUResourcePool::get(Handle<GPUBuffer> handle)
    {
        return m_gpuBufferPool->get(handle);
    }
    Handle<GPUBuffer> GPUResourcePool::set(GPUBuffer &item)
    {
        return m_gpuBufferPool->set(item);
    }

    bool GPUResourcePool::clear(Handle<GPUBuffer> handle, GPUBuffer &item)
    {
        return m_gpuBufferPool->clear(handle, item);
    }

    bool GPUResourcePool::clear(Handle<GPUBuffer> handle)
    {
        return m_gpuBufferPool->clear(handle);
    }

};
