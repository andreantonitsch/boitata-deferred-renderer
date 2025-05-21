#pragma once
#include <memory>

#include <boitatah/modules/Camera.hpp>
#include <boitatah/resources/GPUBuffer.hpp>
#include <boitatah/modules/GPUResourceManager.hpp>

namespace boitatah {

    class BufferedCamera : public Camera{

        public :
            BufferedCamera(const CameraDesc& desc, 
                           std::shared_ptr<GPUResourceManager> manager) : 
                                Camera(desc), 
                                m_manager(manager){
                auto m = std::shared_ptr<GPUResourceManager>(m_manager);
                m_buffer = m->create(GPUBufferCreateDescription{
                                        .size = sizeof(CameraUniforms),
                                        .usage = BUFFER_USAGE::UNIFORM_BUFFER,
                                        .sharing_mode = SHARING_MODE::EXCLUSIVE,
                });
            };
            ~BufferedCamera(){
                auto manager = std::shared_ptr<GPUResourceManager>(m_manager);
                manager->destroy(m_buffer);
            };
            Handle<GPUBuffer> getCameraBuffer(){
                auto manager = std::shared_ptr<GPUResourceManager>(m_manager);
                auto& buff = manager->getResource(m_buffer);
                auto uniforms = getCameraUniforms();
                buff.copyData(&uniforms, sizeof(CameraUniforms));
                return m_buffer;
            };
        private :
            Handle<GPUBuffer> m_buffer;
            std::weak_ptr<GPUResourceManager> m_manager;
    };


}