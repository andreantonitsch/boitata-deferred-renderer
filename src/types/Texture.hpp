#pragma once

#include <memory>
#include <vector>
#include <renderer/resources/GPUResource.hpp>
#include <types/BttEnums.hpp>
#include <types/Image.hpp>

#include <collections/Pool.hpp>
#include <renderer/resources/GPUBuffer.hpp>
#include <renderer/resources/ResourceStructs.hpp>
//#include <renderer/modules/GPUResourceManager.hpp>

namespace boitatah{

    //structs
    struct SamplerData{

        SAMPLER_FILTER magMip = SAMPLER_FILTER::LINEAR;
        SAMPLER_FILTER minMip = SAMPLER_FILTER::LINEAR;
        SAMPLER_MIPMAP_MODE mipmap = SAMPLER_MIPMAP_MODE::LINEAR;
        SAMPLER_TILE_MODE u_tiling = SAMPLER_TILE_MODE::REPEAT;
        SAMPLER_TILE_MODE v_tiling = SAMPLER_TILE_MODE::REPEAT;
        float lodBias = 0;
        bool normalized = true;
    };

    struct Sampler{
        SamplerData data;

    };

    class FixedTexture;
    class RenderTexture;
    class GPUResourceManager;
    class GPUBuffer;

    //GPU data
    struct TextureGPUData{
        Handle<Image> image;
        Handle<Sampler> sampler;
        IMAGE_FORMAT format;
        IMAGE_LAYOUT layout;
    };


    template<>
    struct ResourceTraits<FixedTexture>{
        using ContentType = TextureGPUData;
        using CommandBufferWriter = vk::VkCommandBufferWriter;
    };
    template<>
    struct ResourceTraits<RenderTexture>{
        using ContentType = TextureGPUData;
        using CommandBufferWriter = vk::VkCommandBufferWriter;
    };

    struct TextureCreateDescription{
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t channels;
        IMAGE_FORMAT format = IMAGE_FORMAT::RGBA_8_SRGB;
        IMAGE_LAYOUT layout = IMAGE_LAYOUT::UNDEFINED;
        RESOURCE_MUTABILITY mutability = RESOURCE_MUTABILITY::IMMUTABLE;
    };

    class Texture {
        enum class TextureMode{
            WRITE,
            READ,
            ATTACH,
            TRANSFER_DEST,
            TRANSFER_SRC,
            PRESENT
        };
        protected:
            std::vector<Handle<Image>> m_ownedImages;
            Handle<GPUBuffer> m_stagingBuffer;
            IMAGE_LAYOUT m_desiredLayout;
            TextureMode m_mode;
            std::shared_ptr<GPUResourceManager> m_manager;

        public:
            Texture() = default;
            ~Texture() = default;
            Texture(const TextureCreateDescription& description, std::shared_ptr<GPUResourceManager> manager){};
            void copyImageFromBuffer(void *data, uint32_t size);

            void transition(TextureMode mode);


            TextureGPUData CreateGPUData() {return CreateGPUData();};
            bool ReadyForUse(TextureGPUData& content){return true;};
            void SetContent(TextureGPUData& content) {};
            void ReleaseData(TextureGPUData& content) {};
            void Release(TextureGPUData& content);                
            //  {   auto manager = std::shared_ptr<GPUResourceManager>(m_manager);
            // };
            void WriteTransfer(TextureGPUData& data, CommandBufferWriter<vk::VkCommandBufferWriter> &writer) {};
    };

    class FixedTexture : public Texture, public MutableGPUResource<FixedTexture>{
            
            public:
                FixedTexture() = default;
                ~FixedTexture() = default;
                FixedTexture(const FixedTexture& other) = default;

                FixedTexture(const TextureCreateDescription& description, std::shared_ptr<GPUResourceManager> manager);

                TextureGPUData CreateGPUData() 
                    {return Texture::CreateGPUData();};
                bool ReadyForUse(TextureGPUData& content)
                    {return Texture::ReadyForUse(content);};
                void SetContent(TextureGPUData& content)
                    {Texture::SetContent(content);};
                void ReleaseData(TextureGPUData& content)
                    {Texture::ReleaseData(content);};
                void Release(TextureGPUData& content)
                    {Texture::Release(content);};
                void WriteTransfer(TextureGPUData& data, CommandBufferWriter<vk::VkCommandBufferWriter> &writer)
                    {return Texture::WriteTransfer(data, writer);};

    };


    class RenderTexture : public Texture, public MutableGPUResource<RenderTexture>{
            //friend class MutableGPUResource<RenderTexture>;
            public:
                RenderTexture() = default;
                ~RenderTexture() = default;
                RenderTexture(const RenderTexture& other) = default;

                RenderTexture(const TextureCreateDescription &description, std::shared_ptr<GPUResourceManager> manager)
                :           MutableGPUResource<RenderTexture>({ //Base Constructor
                                                    .sharing = SHARING_MODE::EXCLUSIVE,
                                                    .type = RESOURCE_TYPE::TEXTURE,
                                                    .mutability = RESOURCE_MUTABILITY::MUTABLE,
                                                  }, manager) ,
                                        Texture(description, manager){ };
                TextureGPUData CreateGPUData() 
                    {return Texture::CreateGPUData();};
                bool ReadyForUse(TextureGPUData& content)
                    {return Texture::ReadyForUse(content);};
                void SetContent(TextureGPUData& content)
                    {Texture::SetContent(content);};
                void ReleaseData(TextureGPUData& content)
                    {Texture::ReleaseData(content);};
                void Release(TextureGPUData& content)
                    {Texture::Release(content);};
                void WriteTransfer(TextureGPUData& data, CommandBufferWriter<vk::VkCommandBufferWriter> &writer)
                    {return Texture::WriteTransfer(data, writer);};
    };


};
