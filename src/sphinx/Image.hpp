
#pragma once
#include <sphinx/AES.hpp>

namespace sphinx {

    struct Image_Message {
        int count;
        u8* data;

        Image_Message(int count, u8* data)
        : count(count), data(data) {
        }
    };

    /*
        Image in the main memory.
        @todo: cleanup
    */
    struct Image {
        int id;
        int texture_id;
        s16 width;
        s16 height;
        s32 size;
        u8* data;

        Image(int id = -1, s16 width = 0, s16 height = 0, u8* data = nullptr)
        : id(id), texture_id(-1), width(width), height(height), size(0), data(data) {}

        bool
        is_ready_to_write() const;

        bool
        is_ready_to_render() const;

        bool
        try_write(const Image_Message& message);

        bool
        try_read(Image_Message& msg_buf);
    };

    /*
        Image with the given file_name will be loaded asynchronously, however user immediately
        receives a reference to the image instance which will be updated when the image loading
        is complete. At this time image texture will also be generated such that it can be
        drawn in the UI, however this operation is done on the main thread since only main thread
        has the GL context.
    */
    const Image&
    get_image(std::string image_file_path);

}
