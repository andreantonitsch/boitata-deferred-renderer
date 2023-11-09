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
#include "../types/Framebuffer.hpp"
#include "../types/Memory.hpp"
#include "../types/Image.hpp"
#include "../types/CommandBuffer.hpp"

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

        // Swapchain Methos
        void buildSwapchain(FORMAT scFormat);
        std::vector<Image> getSwapchainImages();
        Image acquireSwapChainImage();


        // Create Objects
        VkShaderModule createShaderModule(const std::vector<char> &bytecode);
        VkFramebuffer createFramebuffer(const FramebufferDescVk &desc);
        VkAttachmentDescription createAttachmentDescription(const AttachmentDesc &attDesc);
        VkRenderPass createRenderPass(const RenderPassDesc &desc);
        Image createImage(const ImageDesc &desc);
        VkImageView createImageView(VkImage image, const ImageDesc &desc);
        VkPipelineLayout createPipelineLayout(const PipelineLayoutDesc &desc);

        // Manage Memory
        VkDeviceMemory allocateMemory(const MemoryDesc &desc);
        void bindImageMemory(VkDeviceMemory memory, VkImage image);
        uint32_t findMemoryIndex(const MemoryDesc &props);

        //Commands
        VkCommandBuffer allocateCommandBuffer(const CommandBufferDesc& desc);
        void recordCommand(const DrawCommandVk& command);
        void resetCommandBuffer(const CommandBuffer buffer);
        void submitCommandBuffer(const CommandBuffer buffer);
        void presentFrame();


        // Sync Methods
        void waitForFrame();

        // Destroy Objects
        void destroyShader(Shader &shader);
        void destroyRenderpass(RenderPass &pass);
        void destroyFramebuffer(Framebuffer &framebuffer);
        void destroyImage(Image image);
        void destroyPipelineLayout(PipelineLayout &layout);
        void buildShader(const ShaderDescVk &desc, Shader& shader);

        // Copy assignment?
        // Vulkan& operator= (const Vulkan &v);//copy assignment
    private:
        VulkanOptions options;

        // Instances and Devices
        VkInstance instance;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger;

        VkDevice device; // Logical Device

        // Queues and Pools
        VkCommandPool commandPool;
        VkQueue graphicsQueue;
        VkQueue presentQueue;


        // Swapchain responsabilities
        VkSurfaceKHR surface;

        VkSemaphore SemImageAvailable;
        VkSemaphore SemRenderFinished;
        VkFence FenInFlight;

        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainViews;
        VkFormat swapchainFormat;
        VkExtent2D swapchainExtent;
        std::vector<Image> swapchainImageCache;

        // Extensions and Layers
        std::vector<const char *> validationLayers;
        std::vector<const char *> deviceExtensions;
        std::vector<const char *> instanceExtensions;

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
        void createCommandPools();
        // Window Surfaces
        void createSurface(GLFWwindow *window);

        // Sync Objects
        void createSyncObjects();
        void cleanupSyncObjects();
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