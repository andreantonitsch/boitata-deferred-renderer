#include "RenderObjectManager.hpp"

namespace boitatah{
    RenderObjectManager::RenderObjectManager(vk::Vulkan *vk_instance, std::shared_ptr<GPUResourceManager> resourceManager)
                                             : m_resourceManager(resourceManager)
    {

        geometryPool = std::make_unique<Pool<Geometry2>>(new Pool<Geometry2>({
                .size = 1<<16, .name = "uniforms pool"}));

    }

    Handle<Geometry2> boitatah::RenderObjectManager::createGeometry(const GeometryDesc2 &description)
    {
        if(description.indexData.count == 0) throw std::runtime_error("0 indexed geometry buffer");

        Geometry2 geo;
        std::shared_ptr<GPUResourceManager> gpu_res_mngr(m_resourceManager);
        for (auto& bufferData : description.bufferData)
        {
            if(bufferData.vertexCount == 0) throw std::runtime_error("0 sized vertex buffer");

            auto bufferAddress = gpu_res_mngr->create({
                            .sharing = SHARING_MODE::EXCLUSIVE,
                            .usage = BUFFER_USAGE::TRANSFER_DST_VERTEX,
                            .size = bufferData.vertexSize * bufferData.vertexCount,
                            });
            
            gpu_res_mngr->update(
                bufferAddress,
                bufferData.vertexDataPtr,
                bufferData.vertexCount * bufferData.vertexSize
                );

            geo.buffers.push_back({
                .buffer = bufferAddress,
                .count = bufferData.vertexCount,
                .elementSize = bufferData.vertexSize
            });
        }

        uint32_t data_size = description.indexData.count * sizeof(uint32_t);
        geo.indexBuffer = gpu_res_mngr->create({
            .sharing = SHARING_MODE::EXCLUSIVE, 
            .usage = BUFFER_USAGE::TRANSFER_DST_INDEX,
            .size = data_size,
            });
        gpu_res_mngr->update(geo.indexBuffer, 
                             description.indexData.dataPtr, 
                             data_size);

        geo.indiceCount = description.indexData.count;
        geo.vertexInfo = description.vertexInfo;

        return geometryPool->set(geo);
    }

    bool RenderObjectManager::destroy(Handle<Geometry2> handle)
    {
        Geometry2 geo;
        // if(!geometryPool->tryGet(handle, geo))
        //     if()

        return false;
    }
}
