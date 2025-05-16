cmake_minimum_required(VERSION 3.9.1)
project(DeferredRenderer)
set(CMAKE_CXX_STANDARD 23)


set(OUTPUT_FOLDER ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_FOLDER}/examples)

set (THIRD_PARTY_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/third_party/)

set (SPIRV_Headers_SOURCE_DIR ${THIRD_PARTY_INCLUDE_PATH}/SPIRV-Headers)
add_subdirectory(${SPIRV_Headers_SOURCE_DIR})

add_subdirectory(src/)

add_executable(multi_write_attachment src/examples/multi_write_attachment.cpp)
add_executable(deferred_renderer src/examples/deferred_renderer.cpp)
add_executable(lit_deferred_renderer src/examples/lit_deferred_renderer.cpp)
add_executable(lit_deferred_renderer2 src/examples/lit_deferred_renderer2.cpp)


#-lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
#include(FindVulkan)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(LIBS Vulkan::Vulkan
         glfw
         glm
         boitatah)

target_link_libraries(multi_write_attachment ${LIBS})
target_link_libraries(deferred_renderer ${LIBS})
target_link_libraries(lit_deferred_renderer ${LIBS})
target_link_libraries(lit_deferred_renderer2 ${LIBS})

