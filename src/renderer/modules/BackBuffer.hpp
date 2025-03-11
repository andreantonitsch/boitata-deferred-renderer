#pragma once
// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec2.hpp>
#include <memory>
#include <vulkan/vulkan.h>
#include <vector>
#include "../../types/RenderTarget.hpp"
#include "../../types/BackBufferDesc.hpp"
#include <renderer/modules/RenderTargetManager.hpp>
#include <renderer/modules/MaterialManager.hpp>
#include <types/Material.hpp>
#include <types/Descriptors.hpp>
namespace boitatah{

    class BackBufferManager{
        public:
        BackBufferManager(std::shared_ptr<RenderTargetManager> targetManager);
        // BackBufferManager(std::shared_ptr<RenderTargetManager> targetManager,
        //                   std::shared_ptr<MaterialManager> materialManager);
        ~BackBufferManager(void);

        void setup(BackBufferDesc &desc);
        void setup2(BackBufferDesc2 &desc);
        Handle<RenderPass> getRenderPass();
        Handle<RenderTarget> getNext();
        Handle<RenderTarget> getCurrent();
        uint32_t getCurrentIndex();

        std::vector<Handle<RenderStage>> getNext_Graph();
        RenderStage& getStage(Handle<RenderStage> &handle);
        Handle<RenderTarget> getPresentTarget();
        uint32_t getPresentTargetIndex();

        private:

            void addRenderStageToGraph(RenderStageDesc& desc, glm::vec2 dimensions, std::vector<Handle<RenderStage>>& graph);

            std::unique_ptr<Pool<RenderStage>> m_stagePool;
            std::shared_ptr<ImageManager> m_imageManager;
            std::shared_ptr<RenderTargetManager> m_renderTargetManager;
            //std::shared_ptr<MaterialManager> m_materialManager;
            std::array<std::vector<Handle<RenderStage>>, 2> graphs;
            std::vector<Handle<RenderTarget>> buffers;
            PresentLink present_link;
            Handle<RenderPass> renderpass;
            int current;
            void clearBackBuffer();
    };
}
