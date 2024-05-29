#ifndef BOITATAH_DESCPOOLMANAGER_HPP
#define BOITATAH_DESCPOOLMANAGER_HPP

#include "../vulkan/Vulkan.hpp"
#include "../collections/Pool.hpp"
#include <vector>

namespace boitatah {

    using namespace vk;

    struct DescriptorSetRequest{};
    struct DescriptorSet {};
    struct DescriptorSetPool{
        uint16_t bufferSize;
        uint16_t imageSize;
        
        bool fits(DescriptorSetRequest &request) {
            return true;
        };
    };


    class DescriptorPoolManager {

        public:
            Handle<DescriptorSet> getSet(DescriptorSetRequest& request);
        private:
            Vulkan *m_vk;
            std::vector<DescriptorSetPool> currentPools;
            void createPool(DescriptorSetRequest &request);
            size_t findPool(DescriptorSetRequest &request);



    };

}
#endif //BOITATAH_DESCPOOLMANAGER_HPP