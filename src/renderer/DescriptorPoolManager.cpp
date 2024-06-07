#include "DescriptorPoolManager.hpp"

boitatah::DescriptorPoolManager::DescriptorPoolManager(Vulkan* vulkan, uint32_t maximumSets)
{
    m_vk = vulkan;
};

boitatah::DescriptorPoolManager::~DescriptorPoolManager(){
    
    //release all pools
    while(m_pools.size() > 0){
        releasePool(m_pools.size()-1);
    }
}
void boitatah::DescriptorPoolManager::releasePool(size_t index) {
    m_pools[index].release(m_vk);
};
