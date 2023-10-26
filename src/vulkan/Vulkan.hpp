#ifndef BOITATAH_VK_VULKAN_HPP
#define BOITATAH_VK_VULKAN_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
using boitatah::Shader;
using boitatah::ShaderDesc;

namespace boitatah::vk
{

    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool hasFullSupport()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapchainSupport
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct VulkanOptions
    {
        char *appName = nullptr;
        std::vector<const char *> extensions;
        bool useValidationLayers = false;
        bool debugMessages = false;

        // TODO glfw requires instance to create a surface.
        // I would like to keep both things separate.
        GLFWwindow *window;
    };

    class Vulkan
    {
    public:
        // Vulkan();
        Vulkan(VulkanOptions opts);
        ~Vulkan(void);

        void buildSwapchain(FORMAT scFormat);

        VkShaderModule createShaderModule(const std::vector<char> &bytecode);
        void destroyShaderModule(VkShaderModule module);

        void buildShader(const ShaderDesc &desc, Shader& shader);


        // Copy assignment?
        // Vulkan& operator= (const Vulkan &v);//copy assignment
    private:
        VulkanOptions options;

        VkInstance instance;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger;

        VkDevice device; // Logical Device

        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSurfaceKHR surface;

        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> swapchainImages;
        VkFormat swapchainFormat;
        VkExtent2D swapchainExtent;
        std::vector<VkImageView> swapchainViews;

        std::vector<const char *> validationLayers;
        std::vector<const char *> deviceExtensions;
        std::vector<const char *> instanceExtensions;

        void injectRenderPass(const ShaderDesc &desc, Shader& shader);
        void injectPipelineLayout(const ShaderDesc &desc, Shader& shader);
        void injectPSO(const ShaderDesc &desc, Shader& shader);
        
        void destroyPSO(VkPipeline pipeline);
        void destroyPipelineLayout(VkPipelineLayout layout);
        void destroyRenderPass(VkRenderPass pass);

#pragma region Vulkan Setup

        void initInstance();

        // Extensions
        bool checkRequiredExtensions(const std::vector<VkExtensionProperties> &available,
                                     const std::vector<const char *> &required);
        std::vector<VkExtensionProperties> retrieveInstanceAvailableExtensions();

        // Validation Layers
        void initializeDebugMessenger();
        bool checkValidationLayerSupport(const std::vector<const char *> &layers);
        void populateMessenger(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

        // Device
        int evaluatePhysicalDevice(VkPhysicalDevice device);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        void initPhysicalDevice();
        void initLogicalDeviceNQueues();

        // Queues
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        void setQueues();
        // Window Surfaces
        void createSurface(GLFWwindow *window);

#pragma endregion Vulkan Setup

#pragma region SwapChain
        SwapchainSupport getSwapchainSupport(VkPhysicalDevice device);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats, FORMAT scFormat, COLOR_SPACE scColorSpace);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availableModes);
        void createSwapchain(FORMAT scFormat);
        void createSwapchainViews(FORMAT scFormat);
        void clearSwapchainViews();
#pragma endregion SwapChain

#pragma region Enum Conversion
        template <typename From, typename To>
        static To castEnum(From from);

#pragma endregion Enum Conversion
    };


}

#endif // BOITATAH_VK_VULKAN_HPP