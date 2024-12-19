
#pragma once

namespace sphinx {

    /*
        Image in the CPU!
    */
    struct Image {
        int id;
        s16 width;
        s16 height;
        u8* data;

        Image(int id = -1, s16 width = 0, s16 height = 0, u8* data = nullptr)
        : id(id), width(width), height(height), data(data) {}
    };

    Image
    load_image(const char* file_name);

}
