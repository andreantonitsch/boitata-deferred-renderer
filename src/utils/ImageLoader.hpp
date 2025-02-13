#pragma once
#include "stb_image.h"
#include <types/Texture.hpp>
#include <string>
#include <vector>
#include <renderer/modules/GPUResourceManager.hpp>

namespace boitatah::util{


    class TextureLoader{
        public:
            static Handle<RenderTexture> loadRenderTexture(std::string filepath, GPUResourceManager& manager){
                
                // int texWidth, texHeight, texChannels;
                // stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
                // uint32_t imageSize = texWidth * texHeight * 4;

                // if (!pixels) {
                //     throw std::runtime_error("failed to load texture image!");
                // }

                // TextureCreateDescription textureCreate;
                // textureCreate.width = texWidth;
                // textureCreate.width = texHeight;
                // textureCreate.depth = 1;

                //auto texture = manager.create(textureCreate);




                //stbi_image_free(pixels);

                return Handle<RenderTexture>();
            };
        
        
    };
};