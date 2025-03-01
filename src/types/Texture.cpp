#include <types/Texture.hpp>
#include <renderer/resources/GPUBuffer.hpp>
#include <renderer/modules/GPUResourceManager.hpp>
#include "Texture.hpp"

namespace boitatah{
    Texture::Texture(const TextureCreateDescription &description, std::shared_ptr<GPUResourceManager> manager)
    {
        m_manager = manager;
        m_desiredLayout = IMAGE_LAYOUT::UNDEFINED;
        texProps.depth = description.depth;
        texProps.channels = description.channels;
        texProps.width = description.width;
        texProps.height = description.height;
        texProps.format = description.format;
        texProps.byteSize = texProps.width * 
                            texProps.height * 
                            texProps.depth * 
                            formatSize(description.format);
        m_mode = description.textureMode;
        if(description.textureMode == TextureMode::READ){
            std::cout << "creating sampled image" << std::endl;
            texProps.usage = IMAGE_USAGE::TRANSFER_DST_SAMPLED;
        }


        auto& imageMngr = m_manager->getImageManager();
        texProps.sampler = imageMngr.createSampler(description.samplerInfo);
    }

    void Texture::copyImageFromBuffer(void *data)
    {
        image_generation++;
        {
                std::cout <<"copying " << data << " to image with  size " << texProps.byteSize << std::endl; 
            if(!m_stagingBuffer){
                m_stagingBuffer = m_manager->create(
                        GPUBufferCreateDescription{
                            .size = texProps.byteSize,
                            .usage = BUFFER_USAGE::TRANSFER_SRC,
                            .sharing_mode = SHARING_MODE::CONCURRENT,
                        });
            }

            auto& buffer = m_manager->getResource(m_stagingBuffer);
            buffer.copyData(data, texProps.byteSize);
        }
    }
    void Texture::transition(TextureMode mode) {
        m_mode = mode;    
    };
    // RenderTexture::RenderTexture(const TextureCreateDescription &description, std::shared_ptr<GPUResourceManager> manager)
    TextureGPUData Texture::CreateGPUData()
    {

        auto& imageMngr = m_manager->getImageManager();
        TextureGPUData data;
        
        data.format = texProps.format;
        data.layout = IMAGE_LAYOUT::UNDEFINED;
        data.sampler = texProps.sampler;
        data.image = imageMngr.createImage({
            .format = data.format,
            .dimensions = glm::u32vec2(texProps.width, texProps.height),
            .mipLevels = texProps.sampler.data.mipLevels,
            .initialLayout = data.layout,
            .usage = texProps.usage,
        });
        data.generation = 0;
        return data;
    }

    bool Texture::ReadyForUse(TextureGPUData &content)
    {
        bool ready = content.format == texProps.format;
        ready &= content.layout == m_desiredLayout;
        ready &= content.sampler.sampler == texProps.sampler.sampler;
        ready &= image_generation == content.generation;
        return ready;
    }
    void Texture::ReleaseData(TextureGPUData &content)
    {
        auto& imageMngr = m_manager->getImageManager();
        imageMngr.destroyImage(content.image);
    }
    void Texture::Release()
    {
        auto& imageMngr = m_manager->getImageManager();
        imageMngr.destroySampler(texProps.sampler);
    }
    void Texture::WriteTransfer(TextureGPUData &data, CommandBufferWriter<vk::VkCommandBufferWriter> &writer)
    {

        //copy image from buffer
        auto buffer = m_manager->getResource(m_stagingBuffer).getAccessData(0);

        IMAGE_LAYOUT srcLayout = data.layout;
        if(m_mode == TextureMode::READ){
            std::cout << "changing image to layout read" << std::endl;
            data.layout = IMAGE_LAYOUT::READ;
        }
        
        writer.copyBufferToImage({
            .buffer = buffer.buffer->getBuffer(),
            .image = m_manager->getImageManager().getImage(data.image).image,
            .buffOffset = buffer.offset,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .offset = {0, 0, 0},
            .extent = {texProps.width, texProps.height, 1},
            .srcImgLayout = castEnum<VkImageLayout>(srcLayout),
            .dstImgLayout = castEnum<VkImageLayout>(data.layout),
        });

    }

    TextureAccessData RenderTexture::getAccessData(uint32_t frame_index)
    {
        auto manager = std::shared_ptr<GPUResourceManager>(MutableGPUResource<RenderTexture>::m_manager);
        auto& res = resource().get_content(frame_index);
        auto& image = manager->getImageManager().getImage(res.image);
            //resource().get_content(frame_index).image
        return {.image = image.view,
                .layout = castEnum<VkImageLayout>(res.layout),
                .sampler = res.sampler.sampler,
                };
    }
};