#pragma once
#include <renderer/resources/GPUResource.hpp>

namespace boitatah{


    class Texture;
    class GPUResourceManager;

    struct TextureGPUData;
    
    template<>
    struct ResourceTraits<Texture>{
        using ContentType = TextureGPUData;
        using CommandBufferWriter = vk::VkCommandBufferWriter;

    };

    struct TextureGPUData{
        Handle<Image> image;
        IMAGE_FORMAT format;
        IMAGE_LAYOUT layout;
    };

    struct TextureMetaData : ResourceMetaContent<Texture>{
        Handle<Image> image;
    };


    class Texture {
        
    };

    class FixedTexture : public ImmutableGPUResource<Texture>, public Texture{
        

    };

    class RenderTexture : public MutableGPUResource<Texture>, public  Texture{

    };
};
