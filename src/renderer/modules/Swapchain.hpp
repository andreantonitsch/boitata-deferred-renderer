#pragma once

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>
#include <memory>


#include <vulkan/vulkan.h>
#include "../../vulkan/Vulkan.hpp"
#include "../../vulkan/Window.hpp"
#include "../Renderer.hpp"
#include "../../collections/Pool.hpp"
#include <vector>
#include <string>
#include "../../types/RenderTarget.hpp"
#include "../../types/BttEnums.hpp"

namespace boitatah
{
    using namespace vk;

    class Renderer;

    struct SwapchainOptions
    {
        FORMAT format;
        bool useValidationLayers = false;
    };

    struct SwapchainImage{
        Image image;
        uint32_t index;
        VkSwapchainKHR sc;
    };

    class Swapchain
    {
    public:
        Swapchain(SwapchainOptions options);
        ~Swapchain(void);
        SwapchainImage getNext(VkSemaphore &semaphore);
        SwapchainImage getCurrent();
        void attach(std::shared_ptr<Vulkan> vulkan, Renderer *renderer, std::shared_ptr<WindowManager> window);
        void createSwapchain();//Vector2<uint32_t> dimensions, bool vsync, bool fullscreen);
        // void populateBuffers();

    private:
        SwapchainOptions options;
        //VkSurfaceKHR surface;
        std::shared_ptr<vk::Vulkan> vulkan;
        Renderer *renderer;
        std::shared_ptr<WindowManager> window;
        uint32_t currentIndex;

        // Swapchain responsabilities
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainViews;
        VkFormat swapchainFormat;
        VkExtent2D swapchainExtent;
        std::vector<Image> swapchainImageCache;

        std::vector<Image> getSwapchainImages();

        void createVkSwapchain();
        void createViews();

        void clearSwapchainViews();

        //Query and Choose swapchain parameters
        SwapchainSupport getSwapchainSupport(VkPhysicalDevice physicalDevice);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats,
            FORMAT scFormat,
            COLOR_SPACE scColorSpace);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availableModes);
        
    };
}

