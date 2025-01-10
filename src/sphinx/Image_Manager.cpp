
// Implements:
#include <sphinx/Image_Manager.hpp>

// Dependencies:
#include <util/Thread_Pool.hpp>
#include <util/Circular_Vector.hpp>

// // Dependencies:
// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// #define STB_IMAGE_WRITE_IMPLEMENTATION
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
        static constexpr int MIN_CAPACITY { 32 };

    private:
        std::unordered_map<std::string, PNG_Image> images;
        Thread_Pool                                thread_pool;
        Circular_Vector<std::string>               images_to_load;

    private:
        void
        load_image_from_disk(const std::string& file_path) {
            PNG_Image& image = images[file_path];
            EXPECT(!image.is_loaded());

            std::cout << "Loading from disk: " << file_path << std::endl;

            image.data = stbi_load(
                file_path.c_str(),
                (int*)&image.width,
                (int*)&image.height,
                NULL,
                PNG_Image::CHANNELS
            );

            EXPECT(image.width > 0 && image.height > 0 && image.data != nullptr);
            images[file_path] = image;
        }

        void
        load_image_task(Stop_Flag& should_stop, int index) {
            while (!should_stop) {
                if (images_to_load.is_empty()) {
                    sleep_for(16);
                }

                std::string& image_path = images_to_load.read();
                if (image_path.size() > 0) {
                    std::cout << "size: " << images_to_load.get_count() << std::endl;
                    load_image_from_disk(image_path);
                }
            }

            std::cout << "thread done: "<< index<<std::endl;
            sleep_for(128);
        }

    public:
        Image_Manager_Context(): images_to_load(MIN_CAPACITY*2) {
            images.reserve(Image_Manager_Context::MIN_CAPACITY);

            // Start the worker threads:
            // for (int i = 0; i < 1; ++i) {
                thread_pool.start_thread(
                    [](Stop_Flag& stop_flag, int index, Image_Manager_Context* context) {
                        context->load_image_task(stop_flag, index);
                    },
                    this
                );
            // }
        }

        PNG_Image&
        request_image(const std::string& image_path) {
            PNG_Image& image = images[image_path];

            // Add to the load queue once:
            if (image.width < 0) {
                image.width = 0;

                images_to_load.write(image_path);
            }

            return image;
        }

        void
        create_texture(PNG_Image& image) {
            // MAIN_THREAD_ONLY();
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
                GL_RGBA, // We are only doing .PNG, because why not...
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

}