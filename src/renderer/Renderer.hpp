#ifndef BOITATAH_RENDERER_HPP
#define BOITATAH_RENDERER_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>

#include "../types/Vector.hpp"
#include "../vulkan/Vulkan.hpp"
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../types/Framebuffer.hpp"
#include "../collections/Pool.hpp"
#include "../types/CommandBuffer.hpp"
#include "../types/Scene.hpp"


// Objective here is to have expose no lone vulkan types.
// so that we can manage them. Thats what the vulkan class is for.
// Renderer manages and exposes them
// from a higher-level point of view

namespace boitatah
{
    using namespace vk;

    template class Pool<Shader>;
    template class Pool<Framebuffer>;
    template class Pool<RenderPass>;
    template class Pool<Image>;

    struct RendererOptions
    {
        Vector2<uint32_t> windowDimensions = {800, 600};
        const char *appName = "Window";
        bool debug = false;
        FORMAT swapchainFormat = FORMAT::BGRA_8_SRGB;
        // FORMAT renderpassColorFormat = FORMAT::RGBA_8_SRGB;
    };

    class Renderer
    {
    public:
        // Constructors / Destructors
        Renderer(RendererOptions options);
        ~Renderer(void);

        // Window Methods
        bool isWindowClosed();


        // Render Methods
        void render(SceneNode &scene, Handle<Framebuffer> &rendertarget);
        void writeCommandBuffer(SceneNode &scene, Handle<Framebuffer> &rendertarget);
        void present(Handle<Framebuffer> &rendertarget);


        // Object Creation
        // Creates PSO object, shader + pipeline.
        // Needs a Framebuffer for compatibility.
        Handle<Shader> createShader(const ShaderDesc &data);
        // Creates a framebuffer with a renderpass.
        Handle<Framebuffer> createFramebuffer(const FramebufferDesc &data);
        Handle<RenderPass> createRenderPass(const RenderPassDesc &data);
        Handle<Image> createImage(const ImageDesc &desc);
        Handle<PipelineLayout> createPipelineLayout(const PipelineLayoutDesc &desc);
        
        CommandBuffer allocateCommandBuffer(const CommandBufferDesc &desc);
        void recordCommand(const DrawCommand& command);

        void destroyShader(Handle<Shader> shader);
        void destroyFramebuffer(Handle<Framebuffer> buffer);
        void destroyRenderPass(Handle<RenderPass> pass);
        void destroyLayout(Handle<PipelineLayout> layout);

    private:

        // Window Methods
        void initWindow();
        void buildSwapchain();
        // Members
        CommandBuffer drawBuffer;
        
        
        std::vector<Handle<Framebuffer>> swapchainBuffers;
        
        // Pools
        //  Pool<RenderTarget> renderTargetPool;
        Pool<Shader> shaderPool = Pool<Shader>({.size = 100});
        Pool<Framebuffer> frameBufferPool = Pool<Framebuffer>({.size = 50});
        Pool<RenderPass> renderpassPool = Pool<RenderPass>({.size = 50});
        Pool<Image> imagePool = Pool<Image>({.size = 500});
        Pool<PipelineLayout> pipelineLayoutPool = Pool<PipelineLayout>({.size = 50});

        // Base objects
        // if this is a value member, then i have to deal with member initialization
        // This being a reference makes the code simpler for now
        // this however is not ideal
        Vulkan *vk;
        GLFWwindow *window;

        // Options Members
        RendererOptions options;

        // Window Functions
        const std::vector<const char *> requiredWindowExtensions();
        void windowEvents();
        void cleanupWindow();

        // Vulkan Instance
        void createVulkan();

        // Logical and physical devices
        // void initializeDevices();

        // Cleanup Functions
        void cleanup();
        void cleanupSwapchainBuffers();
    };
}
#endif // BOITATAH_RENDERER_HPP