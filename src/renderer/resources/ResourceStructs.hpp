#ifndef BOITATAH_RESOURCE_STRUCTS_HPP
#define BOITATAH_RESOURCE_STRUCTS_HPP

#include "../../types/BttEnums.hpp"

namespace boitatah{

    struct GPUBufferCreateDescription{
        uint32_t size;
        BUFFER_USAGE usage;
        SHARING_MODE sharing_mode;
    };

    enum class RESOURCE_TYPE{
        GPU_BUFFER,
        GEOMETRY,
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
    struct ResourceUpdateDescription{
    };

    template<typename T>
    struct ResourceCreateDescription{

    };

    /// @brief Resource content to be shipped to gpu buffers
    /// @tparam type of resource 
    template<class T>
    struct ResourceGPUContent{ 
        using ContentType = typename ResourceGPUContent<T>::ContentType;
        auto getContent() const -> const ContentType&;
    };

    /// @brief Meta Resources this resource needs or points to,
    ///        p.e. Handles to other resources and//or buffers.
    ///        Not shipped to GPU buffers
    /// @tparam Final type of resource
    template<typename T>
    struct ResourceMetaContent{ };

    template <class T>
    inline auto ResourceGPUContent<T>::getContent() const -> const ContentType&
    {
        return static_cast<ContentType>(this);
    }
}

#endif