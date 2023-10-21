#ifndef BOITATAH_VK_VULKAN_HPP
#define BOITATAH_VK_VULKAN_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

namespace boitatah::vk
{

    struct QueueFamilyIndices{
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool hasFullSupport(){
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct VulkanOptions
    {
        char *appName = nullptr;
        std::vector<const char *> extensions;
        bool useValidationLayers = false;
        bool debugMessages = false;

        // TODO glfw requires instance to create a surface.
        // I would like to keep both things separate.
        GLFWwindow* window; 

    };

    class Vulkan
    {
    public:
        // Vulkan();
        Vulkan(VulkanOptions opts);
        ~Vulkan(void);

        // Copy assignment?
        // Vulkan& operator= (const Vulkan &v);//copy assignment
    private:
        VulkanOptions options;

        VkInstance instance;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger;

        VkDevice device; //Logical Device

        VkQueue graphicsQueue;
        VkQueue presentQueue;
        
        VkSurfaceKHR surface;


        std::vector<const char *> validationLayers;

        void initVkInstance();

#pragma region Bookkeeping-Startup

        // Extensions
        bool checkRequiredExtensions(const std::vector<VkExtensionProperties> &available,
                                     const std::vector<const char *> &required);
        std::vector<VkExtensionProperties> retrieveAvailableExtensions();

        // Validation Layers
        void initializeDebugMessenger();
        bool checkValidationLayerSupport(const std::vector<const char *> &layers);
        void populateMessenger(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

        //Device
        int evaluatePhysicalDevice(VkPhysicalDevice device);
        void initPhysicalDevice();
        void initLogicalDeviceNQueues();

        // Queues
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        //Window Surfaces
        void createSurface(GLFWwindow* window);

#pragma endregion Bookkeeping-Startup

    };

}

#endif //BOITATAH_VK_VULKAN_HPP