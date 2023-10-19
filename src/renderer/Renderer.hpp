#ifndef BOITATAH_RENDERER_HPP
#define BOITATAH_RENDERER_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "../types/Dimension.hpp"
#include "../vulkan/Vulkan.hpp"

namespace boitatah
{
    using namespace vk;

    struct RendererOptions
    {
        Dimension2<uint32_t> windowDimensions = {800, 600};
        const char *appName = "Window";
    };

    class Renderer
    {
    public:
        // Constructors / Destructors
        Renderer(RendererOptions options);
        ~Renderer(void);

        // Methods
        void render();
        void init();
        void initWindow();
        bool isWindowClosed();

    private:
        // Base objects
        Vulkan *vk; // Is this right?
        GLFWwindow *window;

        // Options Members
        RendererOptions options;

        // Window Functions
        const char **requiredWindowExtensions(uint32_t &extensionCount);
        void windowEvents();
        void cleanupWindow();

        // Vulkan Instance
        void createVkInstance();

        void cleanup();
    };
}
#endif // BOITATAH_RENDERER_HPP