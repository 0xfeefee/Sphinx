
#pragma once
/*
   32 bits per channel RGBA image on the CPU.
*/
#include <sphinx/AES.hpp>

namespace sphinx {

    constexpr int MAX_MESSAGE_SIZE_BITS    = BLOCK_SIZE*4*8;

    struct PNG_Image {
        static constexpr int CHANNELS = 4;

        int texture_id { -1 };
        s16 width      { -1 };
        s16 height     { -1 };
        u8* data       { nullptr };

        [[nodiscard]]
        bool
        is_loaded() const;

        [[nodiscard]]
        bool
        is_ready_to_render() const;

        [[nodiscard]]
        bool
        try_write(const std::string& message, const std::string& image_filename);

        void
        clear_header();

        [[nodiscard]]
        std::string
        read();
    };

}