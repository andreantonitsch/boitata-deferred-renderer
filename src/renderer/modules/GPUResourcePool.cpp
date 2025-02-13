#include "GPUResourcePool.hpp"



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

        m_renderTexPool = std::make_unique<Pool<RenderTexture>>(PoolOptions{
            .size = 1024,
            .dynamic = true,
            .name = "render texture pool",
        });

        // m_fixedTexPool = std::make_unique<Pool<FixedTexture>>(PoolOptions{
        //     .size = 1024,
        //     .dynamic = true,
        //     .name = "fixed texture pool",
        // });
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



    RenderTexture& GPUResourcePool::get(Handle<RenderTexture> handle)
    {
        return m_renderTexPool->get(handle);
    }
    Handle<RenderTexture> GPUResourcePool::set(RenderTexture &item)
    {
        return m_renderTexPool->set(item);
    }

    bool GPUResourcePool::clear(Handle<RenderTexture> handle, RenderTexture &item)
    {
        return m_renderTexPool->clear(handle, item);
    }

    bool GPUResourcePool::clear(Handle<RenderTexture> handle)
    {
        return m_renderTexPool->clear(handle);
    }


    // FixedTexture& GPUResourcePool::get(Handle<FixedTexture> handle)
    // {
    //     return m_fixedTexPool->get(handle);
    // }
    // Handle<FixedTexture> GPUResourcePool::set(FixedTexture &item)
    // {
    //     return m_fixedTexPool->set(item);
    // }

    // bool GPUResourcePool::clear(Handle<FixedTexture> handle, FixedTexture &item)
    // {
    //     return m_fixedTexPool->clear(handle, item);
    // }

    // bool GPUResourcePool::clear(Handle<FixedTexture> handle)
    // {
    //     return m_fixedTexPool->clear(handle);
    // }

};
