
// Implements:
#include <sphinx/Image_Manager.hpp>

// Dependencies:
#include <util/Thread_Pool.hpp>
#include <util/SWMR.hpp>

// // Dependencies:
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <GL/gl3w.h>

/*
## Internal: Image_Manager_Context

    State of the { Image_Manager } abstracted out because nobody outside cares about this or
    should care about it.
*/
namespace sphinx {

    class Image_Manager_Context {
    public:
        static constexpr int IMG_LOADERS  { 7 };
        static constexpr int IMG_WRITERS  { 1 };

    private:
        std::unordered_map<std::string, PNG_Image> images;
        Thread_Pool                                thread_pool;
        SWMR_Throwaway<std::string, IMG_LOADERS>   load_requests;
        SWSR_Buffer<std::string>                   save_requests;

    private:
        void
        load_image_task(Stop_Flag& should_stop, int index) {
            while (!should_stop) {
                std::string& image_path = load_requests.read(index);
                if (image_path.size() == 0) {
                    sleep_for(16);
                    continue;
                }

                PNG_Image& image = images[image_path];
                EXPECT(!image.is_loaded());

                image.data = stbi_load(
                    image_path.c_str(),
                    (int*)&image.width,
                    (int*)&image.height,
                    NULL,
                    PNG_Image::CHANNELS
                );

                EXPECT(image.width > 0 && image.height > 0 && image.data != nullptr);
            }
        }

        void
        save_image_task(Stop_Flag& should_stop, int index) {
            while (!should_stop) {
                if (save_requests.is_empty()) {
                    sleep_for(16);
                    continue;
                }

                std::string& image_path = save_requests.read();
                EXPECT(image_path.size() > 0);

                PNG_Image& image = images[image_path];
                EXPECT(image.is_loaded());

                stbi_write_png(
                    image_path.c_str(),
                    image.width,
                    image.height,
                    PNG_Image::CHANNELS,
                    (void*)image.data,
                    image.width*4
                );
            }
        }

    public:
        Image_Manager_Context() {
            // Start the worker threads:
            for (int i = 0; i < IMG_LOADERS; ++i) {
                thread_pool.start_thread(
                    [](Stop_Flag& stop_flag, int index, Image_Manager_Context* context) {
                        context->load_image_task(stop_flag, index);
                    },
                    this
                );
            }

            for (int i = 0; i < IMG_WRITERS; ++i) {
                thread_pool.start_thread(
                    [](Stop_Flag& stop_flag, int index, Image_Manager_Context* context) {
                        context->save_image_task(stop_flag, index);
                    },
                    this
                );
            }
        }

        PNG_Image&
        request_image(const std::string& image_path) {
            PNG_Image& image = images[image_path];

            // Add to the load queue once:
            if (image.width < 0) {
                image.width = 0;

                load_requests.write(image_path);
            }

            return image;
        }

        void
        request_image_save(const std::string& image_path) {
            save_requests.write(image_path);
        }

        void
        create_texture(PNG_Image& image) {
            EXPECT(!image.is_ready_to_render());

            unsigned int texture_id;
            glGenTextures(1, &texture_id);
            glBindTexture(GL_TEXTURE_2D, texture_id);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                image.width,
                image.height,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                image.data
            );
            glBindTexture(GL_TEXTURE_2D, 0);

            image.texture_id = texture_id;
        }
    };

}

namespace sphinx {

    Image_Manager::Image_Manager() {
        context = std::make_unique<Image_Manager_Context>();
    }

    Image_Manager::~Image_Manager() {}

    PNG_Image&
    Image_Manager::get_image(const std::string& image_path) {
        PNG_Image& image = context->request_image(image_path);
        if (image.is_loaded() && !image.is_ready_to_render()) {
            context->create_texture(image);
        }

        return image;
    }

    void
    Image_Manager::save_image(const std::string& image_path) {
        context->request_image_save(image_path);
    }

}