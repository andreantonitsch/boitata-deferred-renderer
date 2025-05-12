#pragma once
// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec2.hpp>
#include <memory>
#include <vulkan/vulkan.h>
#include <vector>
#include "../../types/RenderTarget.hpp"
#include <renderer/modules/BackBufferDesc.hpp>
#include <renderer/modules/RenderTargetManager.hpp>
#include <renderer/modules/MaterialManager.hpp>
#include <types/Material.hpp>
#include <types/Descriptors.hpp>
namespace boitatah{

    
    class BackBufferManager{
        public:

        static BackBufferDesc BasicDeferredPipeline(uint32_t windowWidth, 
                                                    uint32_t windowHeight);

        static BackBufferDesc BasicForwardPipeline(uint32_t windowWidth,
                                                    uint32_t windowHeight);
                                                    
        static BackBufferDesc BasicMultiWriteForwardPipeline(uint32_t windowWidth,
                                                              uint32_t windowHeight,
                                                              uint32_t present_index);
    

        //BackBufferManager(std::shared_ptr<RenderTargetManager> targetManager);
        BackBufferManager(std::shared_ptr<RenderTargetManager>  target_manager,
                           std::shared_ptr<ImageManager>        image_manager,
                           std::shared_ptr<MaterialManager>     material_manager,
                           std::shared_ptr<GPUResourceManager>  resource_manager,
                           std::shared_ptr<Vulkan>              vulkan_instance);
        ~BackBufferManager(void);

        //sets up the initial backbuffer
        void setup(BackBufferDesc &desc);
        //corrects a set up backbuffer with a new description without changing handles.
        void regenerate_backbuffer(BackBufferDesc &desc);

        uint32_t                getCurrentIndex();

        std::vector<Handle<RenderStage>>&           getNext_Graph();
        std::vector<Handle<RenderStage>>&           getCurrent_Graph();
        RenderStage&                                getStage(Handle<RenderStage> &handle);
        RenderStage&                                getStage(uint32_t index);
        Handle<MaterialBinding>                     getStageBinding(Handle<RenderStage> &handle);
        Handle<MaterialBinding>                     getStageBinding(uint32_t stage_index);
        const std::vector<Handle<RenderTexture>>&   getStageTextures(Handle<RenderStage>& handle);

        Handle<RenderTarget>                getPresentTarget();
        uint32_t                            getPresentTargetIndex();
        uint32_t                            getStageCount();

        //gets the first renderstage that fits the stage mask.
        Handle<RenderStage>                 getCompatibleRenderStage(uint32_t stage_mask);
        Handle<RenderStage>                 getCompatibleRenderStages(uint32_t stage_mask);

        

        private:
            //Creates a new RenderStage.
            //Based on `desc` and the BackBuffer dimensions.
            RenderStage create_renderstage( RenderStageDesc& desc, glm::vec2 dimensions, 
                                               std::vector<Handle<RenderStage>>& graph);
            void destroy_renderstage(Handle<RenderStage> &handle);
            void clearBackBuffer();

            std::unique_ptr<Pool<RenderStage>>      m_stagePool;
            std::shared_ptr<ImageManager>           m_imageManager;
            std::shared_ptr<RenderTargetManager>    m_renderTargetManager;
            std::shared_ptr<MaterialManager>        m_material_manager;
            std::shared_ptr<GPUResourceManager>     m_resource_mngr;
            
            std::shared_ptr<Vulkan>                 m_vulkan;

            Handle<Sampler> sampler;
            
            std::array<std::vector<Handle<RenderStage>>, 3> m_graphs;
            std::vector<std::vector<Handle<RenderTexture>>> m_stage_textures;
            std::vector<Handle<MaterialBinding>>            m_stage_bindings;

            PresentLink         present_link;
            Handle<RenderPass>  renderpass;
            uint32_t            current = 0U;
            uint32_t            stage_count = 0U;
    };
}
