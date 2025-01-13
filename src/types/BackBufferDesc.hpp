#pragma once

#include <glm/vec2.hpp>
#include <vector>
#include "BttEnums.hpp"

namespace boitatah{

    /// @brief attachments.size() == attachmentFormats.size()
    struct BackBufferDesc{
        std::vector<ATTACHMENT_TYPE> attachments = {ATTACHMENT_TYPE::COLOR};
        std::vector<FORMAT> attachmentFormats = {FORMAT::RGBA_8_SRGB};
        SAMPLES samples = SAMPLES::SAMPLES_1;
        glm::u32vec2 dimensions = {0, 0};
    };

}
