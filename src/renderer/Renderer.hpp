#pragma once

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <utility>
#include <glm/vec2.hpp>

#include "../vulkan/Vulkan.hpp"
#include "../vulkan/Window.hpp"

#include "../types/BackBufferDesc.hpp"
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../types/commands/CommandBuffer.hpp"
#include "../types/RenderTarget.hpp"
#include "../types/commands/Commands.hpp"
#include "../types/Material.hpp"

#include "../buffers/Buffer.hpp"
#include "../buffers/BufferManager.hpp"

#include "modules/GPUResourceManager.hpp"

#include "modules/BackBuffer.hpp"

#include "modules/Swapchain.hpp"
#include "modules/DescriptorPoolManager.hpp"
#include "modules/Camera.hpp"

#include "../collections/Pool.hpp"

#include "../scene/Scene.hpp"





// Objective here is to have expose no lone vulkan types.
// so that we can manage them. Thats what the vulkan class is for.
// Renderer manages and exposes them
// from a higher-level point of view

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
        FORMAT swapchainFormat = FORMAT::BGRA_8_SRGB;
        BackBufferDesc backBufferDesc;
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
        //RenderObjectManager& getRenderObjectManager();

        // Window methods
        bool isWindowClosed();

        // Sync Methods
        void waitIdle();

        // Render Methods
        void renderToRenderTarget(SceneNode &scene, const Handle<RenderTarget> &rendertarget, uint32_t frameIndex);
        void render(SceneNode &scene);
        void render(SceneNode &scene, Camera &camera);
        void presentRenderTarget(Handle<RenderTarget> &rendertarget);
        void renderSceneNode(SceneNode &scene, Handle<RenderTarget> &rendertarget);
        void renderSceneNode(SceneNode &scene, Camera &camera, Handle<RenderTarget> &rendertarget);

        // Object Creation
        // Creates PSO object, shader + pipeline.
        // Needs a Framebuffer for compatibility.
        Handle<Shader> createShader(const ShaderDesc &data);
        // Creates a framebuffer with a renderpass.
        Handle<RenderTarget> createRenderTarget(const RenderTargetDesc &data);
        Handle<RenderPass> createRenderPass(const RenderPassDesc &data);
        Handle<Image> addImage(Image image);
        Handle<Image> createImage(const ImageDesc &desc);
        Handle<ShaderLayout> createShaderLayout(const ShaderLayoutDesc &desc);

        // Command Buffers
        CommandBuffer allocateCommandBuffer(const CommandBufferDesc &desc);
        void beginBuffer(const BeginBufferCommand &command);
        void submitBuffer(const SubmitBufferCommand &command);
        void clearCommandBuffer(const CommandBuffer &buffer);

        void beginRenderpass(const BeginRenderpassCommand &command);

        // Constructs a transfer queue for uniform updating on the beginning of the frame.
        void queueTransferUniform(const TransferUniformCommand &command);
        void transferImage(const TransferImageCommand &command);
        void copyBuffer(const CopyBufferCommand &command);
        void drawCommand(const DrawCommand &command);


        void bindDummyPipeline();

        //UNIFORMS
        Handle<Uniform> createUniform(void *data, uint32_t size, DESCRIPTOR_TYPE type);
        void updateUniform(const Handle<Uniform> uniform, const void* new_data, const uint32_t new_size);
        void commitSceneNodeUniforms(const SceneNode * scene_nodes);


        Handle<RenderPass> getBackBufferRenderPass();

        Handle<RenderTargetCmdBuffers> createRenderTargetCmdData();

        void destroyShader(Handle<Shader> shader);
        void destroyRenderTarget(Handle<RenderTarget> buffer);
        void destroyRenderPass(Handle<RenderPass> pass);
        void destroyLayout(Handle<ShaderLayout> layout);

    private:
        // Base objects
        // if this is a value member, then i have to deal with member initialization
        // This being a reference makes the code simpler for now
        // this however is not ideal
        DescriptorPoolManager *descriptorPoolManager;
        std::shared_ptr<BufferManager> m_bufferManager;
        std::shared_ptr<VkCommandBufferWriter> m_ResourceManagerTransferWriter;
        std::shared_ptr<Swapchain> m_swapchain;
        std::shared_ptr<BackBufferManager> m_backBufferManager;
        std::shared_ptr<GPUResourceManager> m_resourceManager;
        std::shared_ptr<Vulkan> m_vk;
        std::shared_ptr<WindowManager> m_window;


        // Frame Uniforms
        Handle<BufferAddress> m_cameraUniforms;
        // Handle<Uniform> m_cameraUniforms;
        DescriptorSetLayout m_baseLayout;
        Shader m_dummyPipeline;

        CommandBuffer m_transferCommandBuffer;
        VkFence m_transferFence;

        void handleWindowResize();
        void createSwapchain();

        // Pools
        Pool<Shader> shaderPool = Pool<Shader>({.size = 10, .name = "shader pool"});
        Pool<RenderTarget> renderTargetPool = Pool<RenderTarget>({.size = 50, .name = "render target pool"});
        Pool<RenderPass> renderpassPool = Pool<RenderPass>({.size = 50, .name = "render pass pool"});
        Pool<Image> imagePool = Pool<Image>({.size = 50, .name = "image pool"});
        Pool<ShaderLayout> pipelineLayoutPool = Pool<ShaderLayout>({.size = 50, .name = "pipeline layout pool"});
        Pool<RenderTargetCmdBuffers> rtCmdPool = Pool<RenderTargetCmdBuffers>({.size = 50, .name = "rtcmd buffers pool"});
        Pool<Geometry> geometryPool = Pool<Geometry>({.size = 50, .name = "geometry pool"});
        Pool<Uniform> uniformPool = Pool<Uniform>({.size = 1<<16, .name = "uniforms pool"});


#pragma region UPDATE QUEUES
        std::vector<Handle<Uniform>> uniformUpdateQueue = std::vector<Handle<Uniform>>();
        std::vector<Handle<BufferAddress>> stagingBufferQueue = std::vector<Handle<BufferAddress>>();
#pragma endregion 

        std::vector<Buffer *> buffers; 

        // Options Members
        RendererOptions m_options;

        // Vulkan Instance
        void createVulkan();

        // Cleanup Functions
        void cleanup();
    };
}
//#endif // BOITATAH_RENDERER_HPP