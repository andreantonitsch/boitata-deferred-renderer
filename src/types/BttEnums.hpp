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

}

#endif //BOITATAH_BTT_ENUMS_HPP