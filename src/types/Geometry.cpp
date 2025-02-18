#include "Geometry.hpp"
#include <renderer/modules/GPUResourceManager.hpp>

namespace boitatah{

    void Geometry::Release() {
        
        auto manager = std::shared_ptr<GPUResourceManager>(m_manager);

        for(auto& buffer : m_ownedBuffers ){
            manager->destroy(buffer);
        }
        manager->destroy(indexBuffer);
        }
        void Geometry::ComputeSmoothNormals() {
            std::cout << "Compute smooth normals not implemented yet" << std::endl;
        };
        void Geometry::ComputeFlatNormals() {
            std::cout << "Compute flat normals not implemented yet" << std::endl;
        };
};