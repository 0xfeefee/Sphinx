
#pragma once
/*
   32 bits per channel RGBA image on the CPU.
*/
namespace sphinx {

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
    };

}