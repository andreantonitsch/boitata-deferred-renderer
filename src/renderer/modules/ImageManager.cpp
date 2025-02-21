#include <renderer/modules/ImageManager.hpp>
#include "ImageManager.hpp"


namespace boitatah{

ImageManager::ImageManager(std::shared_ptr<vk::Vulkan> vulkan) : m_vk(vulkan) {

    m_imagePool = std::make_unique<Pool<Image>>(PoolOptions{
        .size = 4096,
        .dynamic = true,
        .name = "Image Manager Image Pool"
    });
}

Handle<Image> ImageManager::createImage(const ImageDesc &description)
{
    Image image = m_vk->createImage(description);
    image.view = m_vk->createImageView(image.image, description);

    return m_imagePool->set(image);
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

    m_vk->destroyImage(image);
}
Image &ImageManager::getImage(Handle<Image> &handle)
{
    return m_imagePool->get(handle);
}
};