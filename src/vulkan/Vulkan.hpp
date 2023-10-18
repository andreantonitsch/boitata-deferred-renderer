#include <vulkan/vulkan.h>

namespace boitatah::vulkan{


    struct VulkanOptions{
        char* appName = nullptr;
        const char** extensions;
        uint32_t extensionsCount = 0;
    };

    class Vulkan{
        public:
            Vulkan();
            Vulkan(VulkanOptions options);
            ~Vulkan(void);

            // Copy assignment?
            //Vulkan& operator= (const Vulkan &v);//copy assignment
        private:
            VkInstance instance;

            //Clean up
            void cleanup();

            //Checks
            //bool validateExtensions(const char* extensions);
    };

}