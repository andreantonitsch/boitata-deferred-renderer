#pragma once

#include <boitatah/collections.hpp>
#include <boitatah/types/Image.hpp>
#include <boitatah/backend/vulkan/Vulkan.hpp>
#include <memory>


namespace boitatah{

    class ImageManager{
        private:
            std::shared_ptr<vk::VulkanInstance> m_vk;
            std::unique_ptr<Pool<Image>> m_imagePool;
            std::unique_ptr<Pool<Sampler>> m_sampler_pool;
            void destroySampler(VkSampler sampler);

        public:
            ImageManager(std::shared_ptr<vk::VulkanInstance> vulkan);
            Handle<Sampler> createSampler(const SamplerData& samplerData);
            Sampler& getSampler(const Handle<Sampler> handle);
            void destroySampler(const Handle<Sampler> sampler);
            void updateSampler(Sampler& sampler);

            Handle<Image> createImage(const ImageDesc &description);
            bool check_image(const Handle<Image> &handle);
            bool contains (Handle<Image>& handle);
            void destroyImage(const Handle<Image>& handle);
            Image& getImage(Handle<Image> &handle);
    };


};