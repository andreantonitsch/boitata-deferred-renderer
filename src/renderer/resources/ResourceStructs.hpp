#pragma once

#include "../../types/BttEnums.hpp"

namespace boitatah{

    struct GeometryCreateDescription;

    struct GPUBufferCreateDescription{
        uint32_t size;
        BUFFER_USAGE usage;
        SHARING_MODE sharing_mode;
    };

    struct TextureCreateDescription;

    enum class RESOURCE_TYPE{
        GPU_BUFFER,
        GEOMETRY,
        TEXTURE,
    };

    enum class RESOURCE_MUTABILITY{
        IMMUTABLE,
        MUTABLE
    };

    struct ResourceDescriptor{
        SHARING_MODE sharing;
        RESOURCE_TYPE type;
        RESOURCE_MUTABILITY mutability;
    };

    template<class T>
    struct ResourceTraits{
    };

}

