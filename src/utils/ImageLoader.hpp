#pragma once
#define STB_IMAGE_IMPLEMENTATION 0
#include <stb_image.h>
#include <types/Texture.hpp>
#include <string>
#include <vector>
#include <renderer/modules/GPUResourceManager.hpp>

namespace boitatah::utils{


    class TextureLoader{
        public:
            static Handle<RenderTexture> loadRenderTexture(const std::string&           filepath,
                                                           IMAGE_FORMAT                 format,
                                                           TextureMode                  mode,
                                                           const SamplerData&           samplerData,
                                                           GPUResourceManager&          manager)
            {
                
                int texWidth, texHeight, texChannels;
                stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
                uint32_t imageSize = texWidth * texHeight * 4;

                if (!pixels) {
                    throw std::runtime_error("failed to load texture image!");
                }

                TextureCreateDescription textureCreate;
                textureCreate.width = texWidth;
                textureCreate.width = texHeight;
                textureCreate.depth = 1;
                textureCreate.format = format;
                textureCreate.samplerInfo = samplerData;
                textureCreate.textureMode;
                auto texture = manager.create(textureCreate);

                auto& tex = manager.getResource(texture);
                tex.copyImageFromBuffer(pixels, imageSize);

                stbi_image_free(pixels);

                return texture;
            };
        
        
    };
};