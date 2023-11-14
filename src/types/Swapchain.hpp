#ifndef BOITATAH_SWAPCHAIN_HPP
#define BOITATAH_SWAPCHAIN_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <vector>
#include "../types/Vector.hpp"
#include <string>
#include <../collections/Pool.hpp"
#include "../vulkan/Vulkan.hpp"
#include "RenderTarget.hpp"

namespace boitatah{

    class Swapchain{
        public:
            Handle<RenderTarget> getNext(); 
            Handle<RenderTarget> getCurrent();
            void initSurface(WindowManager* window);
            void attach(Vulkan* vulkan, Renderer* renderer);
            void create(Vecctor2<uint32_t> dimensions, bool vsync, bool fullscreen);

            

        private:

            // Swapchain responsabilities
            VkSurfaceKHR surface;
            VkSwapchainKHR swapchain = VK_NULL_HANDLE;
            std::vector<VkImage> swapchainImages;
            std::vector<VkImageView> swapchainViews;
            VkFormat swapchainFormat;
            VkExtent2D swapchainExtent;
            std::vector<Image> swapchainImageCache;

            std::vector<Handle<RenderTarget>> swapchainRenderTargets;
            void clearSwapchain();
    };

}

#endif //BOITATAH_SWAPCHAIN_HPP