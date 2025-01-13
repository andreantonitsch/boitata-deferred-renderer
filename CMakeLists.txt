cmake_minimum_required(VERSION 3.9.1)
project(DeferredRenderer)
set(CMAKE_CXX_STANDARD 23)


set(OUTPUT_FOLDER ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_FOLDER}/examples)


add_subdirectory(src/)


add_executable(test_render src/examples/test_render.cpp)
add_executable(test_rendertotarget src/examples/test_rendertotarget.cpp)
add_executable(test_geometry src/examples/test_geometry.cpp)
add_executable(test_partitionlist src/examples/test_partitionlist.cpp)
add_executable(test_buffer_allocator src/examples/test_buffer_allocator.cpp)
add_executable(test_camera src/examples/test_camera.cpp)
add_executable(test_buffer_manager src/examples/test_buffer_manager.cpp)
add_executable(test_object_manager src/examples/test_object_manager.cpp)


#-lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
#include(FindVulkan)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)


set(LIBS vulkan
         glfw
         glm
         boitatah)

target_link_libraries(test_render ${LIBS})
target_link_libraries(test_rendertotarget ${LIBS})
target_link_libraries(test_geometry ${LIBS})
target_link_libraries(test_partitionlist ${LIBS})
target_link_libraries(test_buffer_allocator ${LIBS})
target_link_libraries(test_camera ${LIBS})
target_link_libraries(test_buffer_manager ${LIBS})
target_link_libraries(test_object_manager ${LIBS})

