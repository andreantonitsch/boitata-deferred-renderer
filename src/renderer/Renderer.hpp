#ifndef BOITATAH_RENDERER_HPP
#define BOITATAH_RENDERER_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <glm/vec2.hpp>

#include "Camera.hpp"

#include "../types/BackBufferDesc.hpp"
#include "BackBuffer.hpp"

#include "../vulkan/Vulkan.hpp"
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../types/CommandBuffer.hpp"
#include "../types/RenderTarget.hpp"
#include "../collections/Pool.hpp"
#include "../types/Commands.hpp"
#include "../types/Scene.hpp"
#include "../types/Buffer.hpp"
#include "Window.hpp"
#include "../types/Swapchain.hpp"

// Objective here is to have expose no lone vulkan types.
// so that we can manage them. Thats what the vulkan class is for.
// Renderer manages and exposes them
// from a higher-level point of view

namespace boitatah
{
    using namespace vk;
    using namespace window;

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

        // Window methods
        bool isWindowClosed();

        // Sync Methods
        void waitIdle();

        // Render Methods
        void renderToRenderTarget(const SceneNode &scene, const Handle<RenderTarget> &rendertarget);
        void render(SceneNode &scene);
        void render(SceneNode &scene, Camera &camera);
        void presentRenderTarget(Handle<RenderTarget> &rendertarget);
        void renderSceneNode(SceneNode &scene, Handle<RenderTarget> &rendertarget);
        void renderSceneNode(SceneNode &scene, Camera& camera, Handle<RenderTarget> &rendertarget);

        // Command Buffers
        CommandBuffer allocateCommandBuffer(const CommandBufferDesc &desc);
        void recordDrawCommand(const DrawCommand &command);
        void clearCommandBuffer(const CommandBuffer &buffer);
        void transferImage(const TransferImageCommand &command);
        void copyBuffer(const CopyBufferCommand &command);

        void beginBuffer(const BeginBufferCommand &command);
        void beginRenderpass(const BeginRenderpassCommand &command);
        void submitBuffer(const SubmitBufferCommand &command);
        void drawCommand(const DrawCommand &command);
        void bindUniformsCommand(const BindUniformsCommand &command);
        void bindPipelineCommand(const BindPipelineCommand &command);
        void bindDummyPipeline();
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
        Handle<Geometry> createGeometry(const GeometryDesc &desc);

        Buffer *createBuffer(const BufferDesc &desc);
        Handle<BufferReservation> reserveBuffer(const BufferReservationRequest &request);
        Handle<BufferReservation> uploadBuffer(const BufferUploadDesc& desc);
        void copyDataToBuffer(const CopyDataToBufferDesc& desc);
        
        void unreserveBuffer(Handle<BufferReservation> &reservation);
        
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
        Vulkan *m_vk;
        WindowManager *m_window;
        BackBufferManager *m_backBufferManager;
        Swapchain *swapchain;

        // Frame Uniforms
        Handle<BufferReservation> m_cameraUniforms;
        // Handle<Uniform> m_cameraUniforms;
        DescriptorSetLayout m_baseLayout;
        Shader m_dummyPipeline;
        
        CommandBuffer m_transferCommandBuffer;
        VkFence m_transferFence;
        
        void handleWindowResize();
        void createSwapchain();

        // Buffers
        Buffer *findOrCreateCompatibleBuffer(const BufferCompatibility &compatibility);
        uint32_t findCompatibleBuffer(const BufferCompatibility &compatibility);

        // Pools
        Pool<Shader> shaderPool = Pool<Shader>({.size = 10, .name = "shader pool"});
        Pool<RenderTarget> renderTargetPool = Pool<RenderTarget>({.size = 50, .name = "render target pool"});
        Pool<RenderPass> renderpassPool = Pool<RenderPass>({.size = 50, .name = "render pass pool"});
        Pool<Image> imagePool = Pool<Image>({.size = 50, .name = "image pool"});
        Pool<ShaderLayout> pipelineLayoutPool = Pool<ShaderLayout>({.size = 50, .name = "pipeline layout pool"});
        Pool<RenderTargetCmdBuffers> rtCmdPool = Pool<RenderTargetCmdBuffers>({.size = 50, .name = "rtcmd buffers pool"});
        Pool<Geometry> geometryPool = Pool<Geometry>({.size = 50, .name = "geometry pool"});
        Pool<BufferReservation> bufferReservPool = Pool<BufferReservation>({.size = 100, .name = "reservation pool"});


        std::vector<Buffer *> buffers;

        // Options Members
        RendererOptions m_options;

        // Vulkan Instance
        void createVulkan();

        // Cleanup Functions
        void cleanup();
    };
}
#endif // BOITATAH_RENDERER_HPP