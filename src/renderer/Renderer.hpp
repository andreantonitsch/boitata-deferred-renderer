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

    struct RendererOptions
    {
        Vector2<uint32_t> windowDimensions = {800, 600};
        const char *appName = "Window";
        bool debug = false;
        FORMAT swapchainFormat = FORMAT::BGRA_8_SRGB;
        //FORMAT renderpassColorFormat = FORMAT::RGBA_8_SRGB;
    };

    class Renderer
    {
    public:
        // Constructors / Destructors
        Renderer(RendererOptions options);
        ~Renderer(void);

        // Methods
        void render();
        void initWindow();
        bool isWindowClosed();
        void buildSwapchain();

        // Object Creation
        // RenderTarget createRenderTarget();
        // RenderTarget get(Handle<RenderTarget> target);
        // void clear(Handle<RenderTarget> target);

        //Creates PSO object, shader + pipeline.
        //Needs a Framebuffer for compatibility.
        Handle<Shader> createShader(ShaderDesc data);
        //Creates a framebuffer with a renderpass.
        Handle<Framebuffer> createFramebuffer(FramebufferDesc data);
        Handle<RenderPass> createRenderPass(RenderPassDesc data);
        void destroyShader(Handle<Shader> shader);
        void destroyFramebuffer(Handle<Framebuffer> buffer);
        void destroyRenderPass(Handle<RenderPass> pass);

    private:

        std::vector<Framebuffer> frameBuffers;

        //Pools
        // Pool<RenderTarget> renderTargetPool;
        Pool<Shader> shaderPool = Pool<Shader>({.size = 100});
        Pool<Framebuffer> frameBufferPool = Pool<Framebuffer>({.size = 50});
        Pool<RenderPass> renderpassPool = Pool<RenderPass>({.size = 50});
        
        // Base objects
        // if this is a value member, then i have to deal with member initialization
        //This being a reference makes the code simpler for now
        // this however is not ideal
        Vulkan *vk;
        GLFWwindow *window;

        // Options Members
        RendererOptions options;

        // Window Functions
        const std::vector<const char*>requiredWindowExtensions();
        void windowEvents();
        void cleanupWindow();

        // Vulkan Instance
        void createVulkan();

        // Logical and physical devices
        //void initializeDevices();

        void cleanup();
    };
}
#endif // BOITATAH_RENDERER_HPP