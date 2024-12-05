#ifndef BOITATAH_RENDERER_OBECT_MANAGER_HPP
#define BOITATAH_RENDERER_OBECT_MANAGER_HPP

#include <memory>

#include "../../vulkan/Vulkan.hpp"
#include "../../collections/Pool.hpp"
#include "../../buffers/Buffer.hpp"
#include "../../buffers/BufferManager.hpp"
#include "../../types/Geometry.hpp"
#include "../../types/GPUResource.hpp"
#include "GPUResourceManager.hpp"

namespace boitatah
{

    class RenderObjectManager{

        public:
            RenderObjectManager(vk::Vulkan* vk_instance, std::shared_ptr<GPUResourceManager> resourceManager); //contructor

            //Handle<Texture> getTexture();
            Handle<Geometry2> createGeometry(const GeometryDesc2 &description);
            
            
            //Handle<RenderBuffer> getRenderBuffer();
            //Handle<RenderFloat> getFloat();
            //Handle<RenderMatrix> getMatrix();
            //Handle<Shader> getShader();

            //maybe material manager
            //Handle<Material> getMaterial();
            //Handle<Bindings> getBindings();

            bool destroy(Handle<Geometry2> handle);

            //template<typename T>
            //bool update<T>(Handle<T> handle);

        private:
            std::weak_ptr<GPUResourceManager> m_resourceManager;
            std::unique_ptr<Pool<Geometry2>> geometryPool; 
    };



};

#endif //BOITATAH_UNIFORM_MANAGER_HPP