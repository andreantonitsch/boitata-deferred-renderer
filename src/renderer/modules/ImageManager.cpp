#include <boitatah/modules/ImageManager.hpp>



namespace boitatah{

ImageManager::ImageManager(std::shared_ptr<vk::VulkanInstance> vulkan) : m_vk(vulkan) {

    m_imagePool = std::make_unique<Pool<Image>>(PoolOptions{
        .size = 4096,
        .dynamic = true,
        .name = "Image Manager Image Pool"
    });
    m_sampler_pool = std::make_unique<Pool<Sampler>>(PoolOptions{
        .size = 4096,
        .dynamic = true,
        .name = "Image Manager Sampler Pool"
    });
}

Handle<Sampler> ImageManager::createSampler(const SamplerData &samplerData)
{   
    Sampler sampler;
    sampler.data = samplerData;
    sampler.sampler = m_vk->create_sampler(samplerData);
    return m_sampler_pool->set(sampler);
}

Sampler &ImageManager::getSampler(const Handle<Sampler> handle)
{
   return m_sampler_pool->get(handle);
}

void ImageManager::destroySampler(Handle<Sampler> sampler)
{
    auto& samp = m_sampler_pool->get(sampler);
    m_vk->destroy_sampler(samp.sampler);
    m_sampler_pool->clear(sampler);
}

Handle<Image> ImageManager::createImage(const ImageDesc &description)
{
    Image image = m_vk->create_image(description);
    if(!description.skip_view)
        image.view = m_vk->create_imageview(image.image, description);

    return m_imagePool->set(image);
}
bool ImageManager::check_image(const Handle<Image> &handle)
{
    return m_imagePool->contains(handle);
}
bool ImageManager::contains(Handle<Image> &handle)
{
    return m_imagePool->contains(handle);
}
void ImageManager::destroyImage(const Handle<Image> &handle)
{
    if(!handle)
        return;
    
    Image image;
    if(!m_imagePool->tryGet(handle, image))
        return;

    m_vk->destroy_image(image);
}
Image &ImageManager::getImage(Handle<Image> &handle)
{
    return m_imagePool->get(handle);
}
};