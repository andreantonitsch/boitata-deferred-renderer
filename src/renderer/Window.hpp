#ifndef BOITATAH_WINDOW_HPP
#define BOITATAH_WINDOW_HPP

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <vector>
#include "../types/Vector.hpp"
#include <string>

namespace boitatah::window{

    struct WindowDesc{
        Vector2<uint32_t> dimensions;
        const char* windowName;
    };

    class WindowManager {
        public:
            WindowManager(WindowDesc& desc);
            ~WindowManager(void);
            bool isWindowClosed();
            void windowEvents();
            const std::vector<const char *> requiredWindowExtensions();
            GLFWwindow *window;

        private:
    };

}

#endif //BOITATAH_WINDOW_HPP