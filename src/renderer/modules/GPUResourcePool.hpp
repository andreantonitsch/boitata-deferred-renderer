#pragma once

#include <memory>
#include <collections/Pool.hpp>
#include <renderer/resources/GPUBuffer.hpp>
#include <renderer/resources/Geometry.hpp>
#include <types/Texture.hpp>
#include <types/Lights.hpp>

namespace boitatah{


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

            RenderTexture& get(Handle<RenderTexture> handle);
            Handle<RenderTexture>  set(RenderTexture& item);
            bool update(Handle<RenderTexture> handle, RenderTexture& item); 
            bool clear(Handle<RenderTexture> handle, RenderTexture& item);
            bool clear(Handle<RenderTexture> handle);

            // FixedTexture& get(Handle<FixedTexture> handle);
            // Handle<FixedTexture>  set(FixedTexture& item);
            // bool update(Handle<FixedTexture> handle, FixedTexture& item); 
            // bool clear(Handle<FixedTexture> handle, FixedTexture& item);
            // bool clear(Handle<FixedTexture> handle);

        private:
            std::unique_ptr<Pool<Geometry>> m_geometryPool;
            std::unique_ptr<Pool<GPUBuffer>> m_gpuBufferPool;
            std::unique_ptr<Pool<RenderTexture>> m_renderTexPool;
            // std::unique_ptr<Pool<FixedTexture>> m_fixedTexPool;

    };

};

