#ifndef BOITATAH_RENDERER_HPP
#define BOITATAH_RENDERER_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>

#include "../types/Dimension.hpp"
#include "../vulkan/Vulkan.hpp"
#include "../types/FORMAT.hpp"
#include "../types/RenderTarget.hpp"
#include "../types/Shader.hpp"
#include "../structures/Pool.hpp"

// Objective here is to have expose no lone vulkan types.
// so that we can manage them. Thats what the vulkan class is for.
// Renderer manages and exposes them
// from a higher-level point of view

namespace boitatah
{
    using namespace vk;
    
    template class Pool<Shader>;

    struct RendererOptions
    {
        Dimension2<uint32_t> windowDimensions = {800, 600};
        const char *appName = "Window";
        bool debug = false;
        FORMAT swapchainFormat = FORMAT::BGRA_8_SRGB;
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
        RenderTarget createRenderTarget();
        RenderTarget get(Handle<RenderTarget> target);
        void clear(Handle<RenderTarget> target);

        Shader createShader(const std::vector<char> code);

    private:
        //Pools
        // Pool<RenderTarget> renderTargetPool;
        Pool<Shader> shaderPool = Pool<Shader>({});
        
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