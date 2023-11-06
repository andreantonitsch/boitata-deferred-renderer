#ifndef BOITATAH_BTT_ENUMS_HPP
#define BOITATAH_BTT_ENUMS_HPP


namespace boitatah{

    enum COLOR_SPACE {
        SRGB_NON_LINEAR = 1,
    };

    enum FORMAT
    {
        RGBA_8_SRGB = 1,
        BGRA_8_SRGB = 2,
        RGBA_8_UNORM = 3,
        BGRA_8_UNORM = 4,
    };

    enum FRAME_BUFFERING {
        NO_BUFFER = 1,
        VSYNC = 2,
        TRIPLE_BUFFER = 3,
    };

    enum SAMPLES{
        SAMPLES_1 = 1,
        SAMPLES_2 = 2,
        SAMPLES_4 = 3,
        SAMPLES_8 = 4,
        SAMPLES_16 = 5
    };

    enum IMAGE_LAYOUT{
        UNDEFINED = 0,
        COLOR_ATT_OPTIMAL = 1,
        PRESENT_SRC = 2,
    };

    enum USAGE {
        TRANSFER_SRC = 1,
        TRANSFER_DST = 2,
        COLOR_ATT = 3,
        DEPTH_STENCIL = 4,
        SAMPLED = 5,
        TRANSFER_DST_SAMPLED = 6,
    };

    enum MEMORY_PROPERTY {
        DEVICE_LOCAL = 1,
        HOST_VISIBLE = 2,
        HOST_COHERENT = 3,
        HOST_CACHED = 4,
        LAZY_ALLOCATE = 5,
        HOST_VISIBLE_COHERENT = 6,

    };
}

#endif //BOITATAH_BTT_ENUMS_HPP