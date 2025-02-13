#include "Geometry.hpp"
#include <renderer/modules/GPUResourceManager.hpp>

namespace boitatah{

    void Geometry::Release() {
        
        auto manager = std::shared_ptr<GPUResourceManager>(m_manager);

        for(auto& buffer : m_ownedBuffers ){
            manager->destroy(buffer);
        }
        manager->destroy(indexBuffer);
        };

};