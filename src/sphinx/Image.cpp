
// Implements:
#include <sphinx/Image.hpp>

// Dependencies:
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <GL/gl3w.h>

/*
@todo: add an SWSR queue or something similar to avoid mutexes...
*/

namespace sphinx {

    constexpr int MESSAGE_HEADER_SIZE = 8;

    bool
    Image::is_ready_to_render() const {
        return texture_id != -1;
    }

    bool
    Image::is_ready_to_write() const {
        return width > 0;
    }

    bool
    Image::try_write(const Image_Message& message) {
        // Total number of bits we need to store this message:
        int message_length_bits = message.count * 8;
        ERROR_IF(message_length_bits + MESSAGE_HEADER_SIZE > size);

        // Write the header: { MESSAGE_SIZE }
        for (int i = 0; i < MESSAGE_HEADER_SIZE; ++i) {
            data[i] &= 0xFE; // Clear the LSB
            data[i] |= (message_length_bits >> (MESSAGE_HEADER_SIZE - 1 - i)) & 1UL;
        }

        for (int i = 0; i < message_length_bits; ++i) {
            data[i + MESSAGE_HEADER_SIZE] &= 0xFE; // Clear the LSB
            data[i + MESSAGE_HEADER_SIZE] |= (message.data[i/8] >> ((message_length_bits - 1 - i) % 8)) & 1;
        }

        return true; // All good!
    }

    bool
    Image::try_read(Image_Message& msg_buf) {
        s32 message_size = 0;

        // Read the header:
        for (int i = 0; i < MESSAGE_HEADER_SIZE; ++i) {
            message_size = (message_size << 1) | data[i] & 1;
        }

        EXPECT(message_size > 0);
        printf("There's a message hidden here, it has exactly: %d bits", message_size)  ;

        return true;
    }

    // image_decode_message(image_data* i)
    // {
    //     u32 message_size = 0;
    //     u8* image_data = i->data;

    //     for (u8 i = 0; i < SG_HEADER_SIZE; i++) {
    //         message_size = (message_size << 1) | (image_data[i] & 1);
    //     }

    //     if (message_size > 256 * 8)
    //     {
    //         return NULL;
    //     }

    //     char* buffer = (char*)malloc(sizeof(char) * message_size + 1);
    //     for (u32 i = 0; i < message_size; i++) {
    //         buffer[i / 8] = (buffer[i / 8] << 1) | (image_data[i + SG_HEADER_SIZE] & 1);
    //     }

    //     buffer[message_size / 8] = '\0';
    //     return buffer;
    // }

    /*
        Images are loaded from disk in a separate thread, when user wants to load an image
        we create a load request, this request will generate a unique id which user can use to
        check if image is loaded.
    */
    struct Image_Load_Request {
        static int last_id;

        int image_id;
        std::string file_name;

        Image_Load_Request()
        : image_id(-1), file_name("") {}

        Image_Load_Request(const std::string& file_name)
        : image_id(last_id++), file_name(file_name) {}
    };

    int Image_Load_Request::last_id = 0;


    /*
        Simplest possible load, at the moment assumes 4 channel 32 bit image.
        @todo: handle failed loads...
    */
    static inline Image
    process_image_load_request(const Image_Load_Request request) {
        constexpr int IMAGE_CHANNELS = 4;

        Image image;
        image.data = stbi_load(
            request.file_name.c_str(),
            (int*)&image.width,
            (int*)&image.height,
            NULL,
            IMAGE_CHANNELS
        );
        EXPECT(image.width > 0 && image.height > 0);
        image.id = request.image_id;

        std::cout << "Loaded: " << image.width << "x" << image.height << " - " << request.file_name << std::endl;
        return image;
    }


    /*
        We do not need to wait to load images, they can be loaded in a separate thread, so we
        use this small helper to do just that.
    */
    class Image_Loader {
    private:
        std::vector<Image_Load_Request>        requests;
        std::unordered_map<std::string, Image> images;
        std::mutex mtx;

        void
        loader_thread_fn() {
            int sleep_time = 0;
            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));

                // Check if there's a new request:
                Image_Load_Request request; {
                    std::lock_guard<std::mutex> lock(mtx);
                    if (!requests.empty()) {
                        request = requests.back();
                        requests.pop_back();
                        sleep_time = 16;
                    } else {
                        sleep_time = 256;
                    }
                }

                // If request is valid, process it:
                if (request.image_id >= 0) {
                    Image image = process_image_load_request(request);

                    // Update the instance!
                    images[request.file_name] = image;
                }
            }
        }

    public:
        Image_Loader() {
            requests.reserve(16);
            images.reserve(32);
        }

        void
        start() {
            std::thread t([this] { loader_thread_fn(); });
            t.detach();
        }

        Image&
        add_request_image(std::string file_name) {
            std::lock_guard<std::mutex> lock(mtx);
            if (images.find(file_name) == images.end()) {
                Image_Load_Request request(file_name);
                requests.push_back(request);
            }

            // This will also create the new image entry, maybe a bit too hacky ...
            return images[file_name];
        }

        const void
        image_load_texture(Image& image) {
            EXPECT(image.texture_id < 0);

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


    // @temporary: dirty singleton ...
    Image_Loader*
    get_loader() {
        static Image_Loader* loader = nullptr;
        if (loader == nullptr) {
            loader = new Image_Loader();
            loader->start();
        }

        return loader;
    }


    const Image&
    get_image(std::string image_file_path) {
        Image& i = get_loader()->add_request_image(image_file_path);

        // Create a texture if image is ready:
        if (!i.is_ready_to_render() && i.is_ready_to_write()) {
            get_loader()->image_load_texture(i);
        }

        return i;
    }

}
