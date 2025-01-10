
#pragma once

// Dependencies:
#include <sphinx/PNG_Image.hpp>

/*
    Easy loading of images from disk in a separate thread, it should be safe to use the
    reference regardless of it's readiness.
*/
namespace sphinx {

    struct Image_Manager_Context;

    class Image_Manager {
    private:
        Unique<Image_Manager_Context> context;

    public:
        Image_Manager();
        ~Image_Manager();

        PNG_Image&
        get_image(const std::string& image_path);
    };

}
