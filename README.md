# boitata-deferred-renderer


Boitatah is a Vulkan based C++ Rendering Library.
It started as a deferred renderer, named after the Brazilian folklore snake-like creature made of fire who illuminates the pampas.

## Requirements

External Dependencies:
Vulkan 1.3
C++ 23
GLM
GLFW3
CMAKE 3.9.1
GLSLC

## Instalation

    cmake CMakeLists.txt
    make all

    The make file will also compile the default shader files for the renderer.
    Move the boitatah_shaders folder to the same path as the executable.
    Move libboitatah.a to your lib folder.

## Use Examples

## Image Examples

A many lights exanple scene:
![alt text](https://github.com/andreantonitsch/boitata-deferred-renderer/raw/main/example_images/many_lights.png "Example 1")


A many objects exanple scene:
![alt text](https://github.com/andreantonitsch/boitata-deferred-renderer/raw/main/example_images/1_light.png "Example 2")

## Roadmap

[ ] Add a default shader custom-path to the Renderer Init.