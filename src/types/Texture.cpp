#include <types/Texture.hpp>
#include <renderer/resources/GPUBuffer.hpp>
#include <renderer/modules/GPUResourceManager.hpp>

namespace boitatah{
    void Texture::copyImageFromBuffer(void *data, uint32_t size)
    {

        {
            if(!m_stagingBuffer)
                m_stagingBuffer = m_manager->create(
                        GPUBufferCreateDescription{
                            .size = size,
                            .usage = BUFFER_USAGE::TRANSFER_SRC,
                            .sharing_mode = SHARING_MODE::CONCURRENT,
                        });

            auto& buffer = m_manager->getResource(m_stagingBuffer);
            buffer.copyData(data, size);
        }
    }
    void Texture::transition(TextureMode mode) {
    
        m_mode = mode;
        switch(mode){

            default:
                break;
        }
    
    };
    // RenderTexture::RenderTexture(const TextureCreateDescription &description, std::shared_ptr<GPUResourceManager> manager)
    // :           MutableGPUResource<RenderTexture>({ //Base Constructor
    //                                                 .sharing = SHARING_MODE::EXCLUSIVE,
    //                                                 .type = RESOURCE_TYPE::TEXTURE,
    //                                                 .mutability = RESOURCE_MUTABILITY::MUTABLE,
    //                                               }, manager) ,
    //                                     Texture(description, manager){ };
};