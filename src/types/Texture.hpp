#pragma once

#include <memory>
#include <vector>
#include <renderer/resources/GPUResource.hpp>
#include <types/BttEnums.hpp>
#include <types/Image.hpp>
#include <types/TextureStructs.hpp>
#include <collections/Pool.hpp>
#include <renderer/resources/GPUBuffer.hpp>
#include <renderer/resources/ResourceStructs.hpp>
//#include <renderer/modules/GPUResourceManager.hpp>

namespace boitatah{

    class FixedTexture;
    class RenderTexture;
    class GPUResourceManager;
    class GPUBuffer;
    class Texture;

    enum class TextureMode{
        WRITE,
        READ,
        ATTACH,
        TRANSFER_DEST,
        TRANSFER_SRC,
        PRESENT
    };

    //GPU data
    struct TextureGPUData{
        Handle<Image> image;
        Sampler sampler;
        IMAGE_FORMAT format;
        IMAGE_LAYOUT layout;
        uint32_t generation;
    };

    template<>
    struct ResourceTraits<Texture>{
        using ContentType = TextureGPUData;
        using CommandBufferWriter = vk::VkCommandBufferWriter;
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
        TextureMode textureMode;
        SamplerData samplerInfo;
    };

    struct TextureProperties{
        uint32_t width;
        uint32_t height;
        uint32_t depth;
        uint32_t channels;
        IMAGE_FORMAT format = IMAGE_FORMAT::RGBA_8_SRGB;
        IMAGE_USAGE usage = IMAGE_USAGE::COLOR_ATT_TRANSFER_DST;
        uint32_t byteSize;
        Sampler sampler;
        
    };



    class Texture {

        protected:
            std::vector<Handle<Image>> m_ownedImages;
            Handle<GPUBuffer> m_stagingBuffer;
            IMAGE_LAYOUT m_desiredLayout;
            TextureMode m_mode;
            std::shared_ptr<GPUResourceManager> m_manager;
            TextureProperties texProps;
            uint32_t image_generation = 0u;
        public:
            Texture() = default;
            ~Texture() = default;
            Texture(const TextureCreateDescription& description, std::shared_ptr<GPUResourceManager> manager);
            void copyImageFromBuffer(void *data);

            void transition(TextureMode mode);
            
            TextureGPUData CreateGPUData();
            bool ReadyForUse(TextureGPUData& content);
            void ReleaseData(TextureGPUData& content);
            void Release();                
            void WriteTransfer(TextureGPUData& data, CommandBufferWriter<vk::VkCommandBufferWriter> &writer);
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
                void ReleaseData(TextureGPUData& content)
                    {Texture::ReleaseData(content);};
                void Release()
                    {Texture::Release();};
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
                :   Texture(description, manager),
                    MutableGPUResource<RenderTexture>({ //Base Constructor
                                                .sharing = SHARING_MODE::EXCLUSIVE,
                                                .type = RESOURCE_TYPE::TEXTURE,
                                                .mutability = RESOURCE_MUTABILITY::MUTABLE,
                                                }, manager) { };
                TextureAccessData getAccessData(uint32_t frame_index); 
                TextureGPUData CreateGPUData() 
                    {return Texture::CreateGPUData();};
                bool ReadyForUse(TextureGPUData& content)
                    {return Texture::ReadyForUse(content);};
                void ReleaseData(TextureGPUData& content)
                    {Texture::ReleaseData(content);};
                void Release()
                    {Texture::Release();};
                void WriteTransfer(TextureGPUData& data, CommandBufferWriter<vk::VkCommandBufferWriter> &writer)
                    {return Texture::WriteTransfer(data, writer);};
    };


};
