#pragma once

#include <collections/Pool.hpp>
#include <types/Image.hpp>
#include <vulkan/Vulkan.hpp>
#include <memory>


namespace boitatah{

    class ImageManager{
        private:
            std::shared_ptr<vk::Vulkan> m_vk;
            std::unique_ptr<Pool<Image>> m_imagePool;
            void destroySampler(VkSampler sampler);

        public:
            ImageManager(std::shared_ptr<vk::Vulkan> vulkan);
            Sampler createSampler(const SamplerData& samplerData);
            void destroySampler(const Sampler sampler);
            void updateSampler(Sampler& sampler);

            Handle<Image> createImage(const ImageDesc &description);
            bool contains (Handle<Image>& handle);
            void destroyImage(const Handle<Image>& handle);
            Image& getImage(Handle<Image> &handle);
    };


};