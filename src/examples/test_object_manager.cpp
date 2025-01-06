#include "../renderer/Renderer.hpp"
#include <iostream>
#include <memory>

// #include <unistd.h>
#include "../types/BttEnums.hpp"
#include "../types/Shader.hpp"
#include "../utils/utils.hpp"
#include "../collections/Pool.hpp"

using namespace boitatah;
using Boitatah = Renderer;
int main()
{

    const uint32_t windowWidth = 1024;
    const uint32_t windowHeight = 768;

    Boitatah r(RendererOptions{
                .windowDimensions = {windowWidth, windowHeight},
                .appName = "Test Frame Buffer",
                .debug = true,
                .swapchainFormat = FORMAT::BGRA_8_UNORM,
                .backBufferDesc = {.attachments = {ATTACHMENT_TYPE::COLOR},
                                   .attachmentFormats = {FORMAT::BGRA_8_UNORM},
                                   .dimensions = {windowWidth, windowHeight}}});


    auto& objManager = r.getResourceManager();


    auto bufferHandle = objManager.create(GPUBufferCreateDescription{
        .size = 1024u,
        .usage = BUFFER_USAGE::TRANSFER_DST_VERTEX,
        .sharing_mode = SHARING_MODE::EXCLUSIVE,
    });

    auto& buffer = objManager.getResource(bufferHandle);

    objManager.destroy(bufferHandle);

    return EXIT_SUCCESS;
}