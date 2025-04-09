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
        texProps.width = description.width;
        texProps.height = description.height;
        texProps.format = description.format;
        texProps.byteSize = texProps.width * 
                            texProps.height * 
                            texProps.depth * 
                            formatSize(description.format);
        m_mode = description.textureMode;

        if(m_mode == TextureMode::READ){
            std::cout << "creating sampled image" << std::endl;
            texProps.usage = IMAGE_USAGE::TRANSFER_DST_SAMPLED;
            m_desiredLayout = IMAGE_LAYOUT::SHADER_READ;
            if( texProps.format == IMAGE_FORMAT::DEPTH_32_SFLOAT ||
                texProps.format == IMAGE_FORMAT::DEPTH_32_SFLOAT_UINT_STENCIL)
                    texProps.format = IMAGE_FORMAT::R_32_SFLOAT;
            if(texProps.format  == IMAGE_FORMAT::DEPTH_24_UNORM_UINT_STENCIL)
                texProps.format == IMAGE_FORMAT::R_32_UINT;
        }

        texProps.channels = formatChannels(texProps.format);
        m_staging_layout = IMAGE_LAYOUT::UNDEFINED;
        auto& imageMngr = m_manager->getImageManager();
        texProps.sampler = imageMngr.createSampler(description.samplerInfo);
        m_staging_image = imageMngr.createImage({
            .format = texProps.format,
            .dimensions = glm::u32vec2(texProps.width, texProps.height),
            .mipLevels = 1,
            .initialLayout = IMAGE_LAYOUT::UNDEFINED,
            .usage = IMAGE_USAGE::STAGING,
            .skip_view = true,
            });
        image_generation = 0U;
        
    }

    void Texture::copyImageFromBuffer(void *data)
    {
        update_from = TextureUpdateFrom::STAGING_BUFFER;
        image_generation++;

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
    void Texture::CmdCopyImageFromImage(Handle<Image> src_image, IMAGE_LAYOUT src_layout)
    {
        
        // TODO check if manager is recording
        update_from = TextureUpdateFrom::STAGING_IMAGE;
        image_generation++;
        
        auto& image = m_manager->getImageManager().getImage(src_image);
        auto& staging_image = m_manager->getImageManager().getImage(m_staging_image);

        if(m_staging_layout != IMAGE_LAYOUT::WRITE)
            m_manager->getCurrentBufferWriter().transitionImage(
                                                {
                                                 .src = castEnum<VkImageLayout>(m_staging_layout),
                                                 .dst = castEnum<VkImageLayout>(IMAGE_LAYOUT::WRITE),
                                                 .image = staging_image.image,
                                                 .srcStage = castEnum<VkPipelineStageFlags>(PIPELINE_STAGE::TOP),
                                                 .dstStage = castEnum<VkPipelineStageFlags>(PIPELINE_STAGE::TRANSFER),
                                                 });
        m_manager->getCurrentBufferWriter().transitionImage(
                                    {
                                        .src = castEnum<VkImageLayout>(src_layout),
                                        .dst = castEnum<VkImageLayout>(IMAGE_LAYOUT::TRANSFER_READ),
                                        .image = image.image,
                                        .srcStage = castEnum<VkPipelineStageFlags>(PIPELINE_STAGE::TOP),
                                        .dstStage = castEnum<VkPipelineStageFlags>(PIPELINE_STAGE::TRANSFER),
                                        });
        m_staging_layout = IMAGE_LAYOUT::WRITE;
        m_manager->getCurrentBufferWriter().copyImage({
            .srcLayout = castEnum<VkImageLayout>(IMAGE_LAYOUT::TRANSFER_READ),
            .dstLayout = castEnum<VkImageLayout>(m_staging_layout),
            .extent = glm::vec2(texProps.width, texProps.height),
            .srcImage = image.image,
            .dstImage = staging_image.image,
        });

        m_manager->getCurrentBufferWriter().transitionImage(
                                                {
                                                 .src = castEnum<VkImageLayout>(m_staging_layout),
                                                 .dst = castEnum<VkImageLayout>(IMAGE_LAYOUT::TRANSFER_READ),
                                                 .image = staging_image.image,
                                                 .srcStage = castEnum<VkPipelineStageFlags>(PIPELINE_STAGE::TRANSFER),
                                                 .dstStage = castEnum<VkPipelineStageFlags>(PIPELINE_STAGE::BOTTOM),
                                                 });
        m_manager->getCurrentBufferWriter().transitionImage(
                                                {
                                                 .src = castEnum<VkImageLayout>(IMAGE_LAYOUT::TRANSFER_READ),
                                                 .dst = castEnum<VkImageLayout>(src_layout),
                                                 .image = image.image,
                                                 .srcStage = castEnum<VkPipelineStageFlags>(PIPELINE_STAGE::TRANSFER),
                                                 .dstStage = castEnum<VkPipelineStageFlags>(PIPELINE_STAGE::BOTTOM),
                                                 });
        m_staging_layout = IMAGE_LAYOUT::TRANSFER_READ;
        // TODO implement image copy
    }
    void Texture::transition(TextureMode mode)
    {
        m_mode = mode;
    }
    TextureAccessData Texture::GetRenderData(TextureGPUData &gpu_data)
    {
        auto& image = m_manager->getImageManager().getImage(gpu_data.image);
        auto& sampler = m_manager->getImageManager().getSampler(gpu_data.sampler);
            //resource().get_content(frame_index).image
        return {.view = image.view,
                .image = image.image,
                .layout = castEnum<VkImageLayout>(gpu_data.layout),
                .sampler = sampler.sampler,
                };
    };
    // RenderTexture::RenderTexture(const TextureCreateDescription &description, std::shared_ptr<GPUResourceManager> manager)
    TextureGPUData Texture::CreateGPUData()
    {

        auto& imageMngr = m_manager->getImageManager();
        TextureGPUData data;
        
        data.format = texProps.format;
        data.layout = IMAGE_LAYOUT::UNDEFINED,
        data.sampler = texProps.sampler;
        auto& sampler = m_manager->getImageManager().getSampler(data.sampler);
        data.image = imageMngr.createImage({
            .format = data.format,
            .dimensions = glm::u32vec2(texProps.width, texProps.height),
            .mipLevels = sampler.data.mipLevels,
            .initialLayout = data.layout,
            .usage = texProps.usage,
        });
        data.generation = 0;
        return data;
    }

    bool Texture::ReadyForUse(TextureGPUData &content)
    {
        bool ready = content.format == texProps.format   && 
                     content.layout == m_desiredLayout   &&
                     content.sampler == texProps.sampler &&
                     content.generation >= image_generation;
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
        
        auto& data_image = m_manager->getImageManager().getImage(data.image);
        switch(m_mode){
            case TextureMode::READ:         m_desiredLayout = IMAGE_LAYOUT::SHADER_READ;break;
            case TextureMode::ATTACH:       m_desiredLayout = IMAGE_LAYOUT::COLOR_ATT ;break;
            case TextureMode::STAGE_LINK:   m_desiredLayout = IMAGE_LAYOUT::SHADER_READ ;break;
            case TextureMode::TRANSFER_DEST:m_desiredLayout = IMAGE_LAYOUT::WRITE;break;
            case TextureMode::TRANSFER_SRC: m_desiredLayout = IMAGE_LAYOUT::TRANSFER_READ;break;
            case TextureMode::PRESENT:      m_desiredLayout = IMAGE_LAYOUT::PRESENT_SRC;break;
            case TextureMode::WRITE:        m_desiredLayout = IMAGE_LAYOUT::WRITE ;break;
        }
        data.layout = m_desiredLayout;

        //copy from buffer
        if( update_from == TextureUpdateFrom::STAGING_BUFFER){
            auto buffer = m_manager->getResourceAccessData(m_stagingBuffer, 0);
            writer.copyBufferToImage({
                .buffer = buffer.buffer->getBuffer(),
                .image = m_manager->getImageManager().getImage(data.image).image,
                .buffOffset = buffer.offset,
                .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
                .offset = {0, 0, 0},
                .extent = {texProps.width, texProps.height, 1},
                .srcImgLayout = castEnum<VkImageLayout>(IMAGE_LAYOUT::UNDEFINED),
                .dstImgLayout = castEnum<VkImageLayout>(m_desiredLayout),});
                data.generation = image_generation;
        }

        //copy from image
        if( update_from == TextureUpdateFrom::STAGING_IMAGE){
            auto& staging_image  = m_manager->getImageManager().getImage( m_staging_image);
            writer.copyImage({
                .srcLayout = castEnum<VkImageLayout>(IMAGE_LAYOUT::TRANSFER_READ),
                .dstLayout = castEnum<VkImageLayout>(m_desiredLayout),
                .extent = {texProps.width, texProps.height},
                .srcImage = staging_image.image,
                .dstImage = data_image.image,});
            data.generation = image_generation;
        }
    }

    TextureAccessData RenderTexture::GetRenderData(uint32_t frame_index)
    {
        return Texture::GetRenderData(resource().get_content(frame_index));        
    }
};