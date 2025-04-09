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

#include "../types/BackBufferDesc.hpp"
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
#include "modules/GPUResourceManager.hpp"
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

    template class Pool<Shader>;
    template class Pool<RenderTarget>;
    template class Pool<RenderPass>;
    template class Pool<Image>;
    class BackBufferManager;
    class Swapchain;

    struct RendererOptions
    {
        glm::u32vec2 windowDimensions = {800, 600};
        const char *appName = "Window";
        bool debug = false;
        IMAGE_FORMAT swapchainFormat = IMAGE_FORMAT::BGRA_8_SRGB;
        BackBufferDesc backBufferDesc;
        BackBufferDesc2 backBufferDesc2;
    };

    class Renderer
    {
    public:
        // Constructors / Destructors
        Renderer(RendererOptions options);

        ~Renderer(void);

        // Manangers
        BufferManager& getBufferManager();
        GPUResourceManager& getResourceManager();
        MaterialManager& getMaterialManager();
        DescriptorSetManager& getDescriptorManager();
        Materials& getMaterials();
        
        BufferedCamera createCamera(const CameraDesc& desc);
        void setLightArray(const Handle<LightArray>& array);


        // Window methods
        bool isWindowClosed();

        // Sync Methods
        void waitIdle();

        // Render Methods
        void write_draw_command(SceneNode &scene, const Handle<RenderTarget> &rendertarget, uint32_t frameIndex);
        void render_graph(SceneNode &scene, BufferedCamera &camera);
        VkSemaphore render_graph_stage(SceneNode            &scene, 
                                        BufferedCamera      &camera, 
                                        Handle<RenderStage> stage,
                                        VkSemaphore         wait_for_last_stage);
        void present_graph(SceneNode    &scene,
                           Camera       &camera);
        void presentRenderTargetNow(Handle<RenderTarget>    &rendertarget,
                                    VkSemaphore             stage_wait,
                                    uint32_t                attachment_index);
        void schedulePresentRenderTarget(Handle<RenderTarget> &rendertarget, uint32_t attachment_index = 0);
    
        // Object Creation
        // Creates PSO object, shader + pipeline.
        // Needs a Framebuffer for compatibility.
        Handle<Shader> createShader(const MakeShaderDesc &data);
        // Creates a framebuffer with a renderpass.
        Handle<RenderTarget> createRenderTarget(const RenderTargetDesc &data);
        Handle<RenderPass> createRenderPass(const RenderPassDesc &data);
        Handle<Image> createImage(const ImageDesc &desc);
        Handle<ShaderLayout> createShaderLayout(const ShaderLayoutDesc &desc);
        //Handle<DescriptorSetLayout> createDescriptorLayout(const DescriptorSetLayoutDesc &desc);
        Handle<Material> createMaterial(const MaterialCreate& description);
        
        // Command Buffers
        CommandBuffer allocateCommandBuffer(const CommandBufferDesc &desc);
        void clearCommandBuffer(const CommandBuffer &buffer);

        void beginRenderpass(const BeginRenderpassCommand &command);


        // Constructs a transfer queue for uniform updating on the beginning of the frame.
        void transferImage(const TransferImageCommand &command);
        void copyBuffer(const CopyBufferCommand &command);
        void drawCommand(const DrawCommand &command);

        void bindVertexBuffers(const BindVertexBuffersCommand& command);
        
        void pushPushConstants(const PushConstantsCommand& command);

        Handle<RenderPass> getBackBufferRenderPass();

        Handle<RenderTargetSync> createRenderTargetCmdData();

        void destroyShader(Handle<Shader>& shader);
        void destroyRenderTarget(Handle<RenderTarget>& buffer);
        void destroyRenderPass(Handle<RenderPass>& pass);
        void destroyLayout(Handle<ShaderLayout>& layout);

    private:
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

        
        //TODO temp member
        Handle<LightArray> lights;

        void handleWindowResize();
        void createSwapchain();

        std::vector<SceneNode*> orderSceneNodes(const std::vector<SceneNode*>& nodes) const;

        // Options Members
        RendererOptions m_options;

        // Vulkan Instance
        void createVulkan();

        // Cleanup Functions
        void cleanup();
    };
}
//#endif // BOITATAH_RENDERER_HPP