cmake_minimum_required(VERSION 3.9.1)
project(DeferredRenderer)
set(CMAKE_CXX_STANDARD 20)


set(OUTPUT_FOLDER ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_FOLDER}/examples)


add_subdirectory(src/)

add_executable(test_window src/examples/test_window.cpp)
add_executable(test_framebuffer src/examples/test_framebuffer.cpp)

#-lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
#include(FindVulkan)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

set(LIBS vulkan
         glfw
         velly)

target_link_libraries(test_window ${LIBS})
target_link_libraries(test_framebuffer ${LIBS})