#pragma once

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <utility>
#include <glm/vec2.hpp>

#include <vulkan/Vulkan.hpp>
#include "../vulkan/Window.hpp"

#include <renderer/modules/BackBufferDesc.hpp>
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../types/commands/CommandBuffer.hpp"
#include "../types/RenderTarget.hpp"
#include "../types/commands/Commands.hpp"
#include "./types/Lights.hpp"
 
#include "../buffers/Buffer.hpp"
#include "../buffers/BufferManager.hpp"

#include <renderer/modules/ImageManager.hpp>
#include <renderer/modules/RenderTargetManager.hpp>
#include <renderer/modules/GPUResourceManager.hpp>
#include "modules/BackBuffer.hpp"
#include "modules/Swapchain.hpp"
#include "modules/Camera.hpp"
#include <renderer/modules/MaterialManager.hpp>
#include <renderer/modules/StageBaseMaterialManager.hpp>
#include <vulkan/DescriptorSetManager.hpp>
#include <renderer/modules/BufferCamera.hpp>
#include "../collections/Pool.hpp"

#include "../scene/Scene.hpp"

namespace boitatah
{
    using namespace vk;
    using namespace buffer;


    class BackBufferManager;
    class Swapchain;

    struct RendererOptions
    {
        glm::u32vec2 windowDimensions = {800, 600};
        const char *appName = "Window";
        bool debug = false;
        IMAGE_FORMAT swapchainFormat = IMAGE_FORMAT::BGRA_8_SRGB;
        BackBufferDesc backBufferDesc;
        BackBufferDesc backBufferDesc2;
    };

    struct RenderObject{
        Handle<Geometry> geometry;
        Handle<Material> material;
    };

    typedef SceneNode<RenderObject>  RenderScene;

    //transform into a template for backendcompatibility with CRTP
    class Renderer
    {
    public:
        // Constructors / Destructors
        Renderer(RendererOptions options);

        ~Renderer(void);

        // Manangers
        BufferManager&          getBufferManager();
        GPUResourceManager&     getResourceManager();
        MaterialManager&        getMaterialManager();
        DescriptorSetManager&   getDescriptorManager();
        Materials&              getMaterials();
        
        BufferedCamera createCamera(const CameraDesc& desc);

        // TODO temp light stuff
        void                setLightArray(const Handle<LightArray>& array);
        Handle<LightArray>  createLightArray(uint32_t size);
        LightArray&         getLightArray(Handle<LightArray> handle);
        
        // Window methods
        bool isWindowClosed();

        // Sync Methods
        void waitIdle();

        // Render Methods
        template<typename T>
        void write_draw_command(CommandBufferWriter<T>      &writer,
                                RenderScene                 &scene,
                                const Handle<RenderTarget>  &rendertarget,
                                uint32_t                    frameIndex);
        void render_graph(std::shared_ptr<RenderScene>      scene,
                                          BufferedCamera    &camera);
        VkSemaphore render_graph_stage(std::shared_ptr<RenderScene>     scene, 
                                                    BufferedCamera      &camera, 
                                                    Handle<RenderStage> stage,
                                                    VkSemaphore         wait_for_last_stage);
        void present_graph(std::shared_ptr<RenderScene> scene,
                           Camera       &camera);
        void presentRenderTargetNow(Handle<RenderTarget>    &rendertarget,
                                    VkSemaphore             stage_wait,
                                    uint32_t                attachment_index);
        //void schedulePresentRenderTarget(Handle<RenderTarget> &rendertarget, uint32_t attachment_index = 0);
    

        void bindVertexBuffers( uint32_t            frame_index, 
                                Handle<Geometry>    geometry, 
                                bool                indexed, 
                                std::vector<VERTEX_BUFFER_TYPE> vertex_buffers,
                                VkCommandBufferWriter           &writer);

        //void bindVertexBuffers(  CommandBufferWriter<BackEndType>& writer);
        
        void pushPushConstants(const PushConstantsCommand& command);

        //Handle<RenderTargetSync> createRenderTargetCmdData();

    private:
        // Options Members
        RendererOptions m_options;

        // Base objects
        std::shared_ptr<BufferManager> m_bufferManager;
        std::shared_ptr<VkCommandBufferWriter> m_buffer_writer;
        std::shared_ptr<Swapchain> m_swapchain;
        std::shared_ptr<BackBufferManager> m_backBufferManager;
        std::shared_ptr<GPUResourceManager> m_resourceManager;
        std::shared_ptr<DescriptorSetManager> m_descriptorManager;
        std::shared_ptr<Vulkan> m_vk;
        std::shared_ptr<WindowManager> m_window;
        std::shared_ptr<MaterialManager> m_materialMngr;
        std::shared_ptr<ImageManager> m_imageManager;
        std::shared_ptr<RenderTargetManager> m_renderTargetManager;
        std::shared_ptr<Materials> m_baseMaterials;

        std::unique_ptr<Pool<LightArray>> m_lightpool;

        //TODO temp member
        Handle<LightArray> lights;

        void handleWindowResize();
        void createSwapchain();

        // Command Buffers
        CommandBuffer allocateCommandBuffer(const CommandBufferDesc &desc);

        std::vector<std::shared_ptr<RenderScene>> 
        orderSceneNodes(const std::vector<std::shared_ptr<RenderScene>> &nodes) const;


        // Vulkan Instance
        void createVulkan();

        // Cleanup Functions
        void cleanup();
    };
}
//#endif // BOITATAH_RENDERER_HPP