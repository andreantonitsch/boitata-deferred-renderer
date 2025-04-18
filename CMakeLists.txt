cmake_minimum_required(VERSION 3.9.1)
project(DeferredRenderer)
set(CMAKE_CXX_STANDARD 23)


set(OUTPUT_FOLDER ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_FOLDER}/examples)

set (THIRD_PARTY_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/third_party/)

set (SPIRV_Headers_SOURCE_DIR ${THIRD_PARTY_INCLUDE_PATH}/SPIRV-Headers)
add_subdirectory(${SPIRV_Headers_SOURCE_DIR})

add_subdirectory(src/)

add_executable(partitionlist src/examples/test_partitionlist.cpp)
add_executable(buffer_allocator src/examples/test_buffer_allocator.cpp)
add_executable(multi_write_attachment src/examples/test_multi_write_attachment.cpp)
add_executable(deferred_renderer src/examples/test_deferred_renderer.cpp)
add_executable(lit_deferred_renderer src/examples/test_lit_deferred_renderer.cpp)


#-lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
#include(FindVulkan)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(LIBS Vulkan::Vulkan
         glfw
         glm
         boitatah)

target_link_libraries(partitionlist ${LIBS})
target_link_libraries(buffer_allocator ${LIBS})
target_link_libraries(multi_write_attachment ${LIBS})
target_link_libraries(deferred_renderer ${LIBS})
target_link_libraries(lit_deferred_renderer ${LIBS})

