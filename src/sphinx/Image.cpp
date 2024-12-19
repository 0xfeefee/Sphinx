
// Implements:
#include <sphinx/Image.hpp>

// Dependencies:
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

/*
@todo: add an SWSR queue or something similar to avoid mutexes...
*/

namespace sphinx {

    /*
        Images are loaded from disk in a separate thread, when user wants to load an image
        we create a load request, this request will generate a unique id which user can use to
        check if image is loaded.
    */
    struct Image_Load_Request {
        static int last_id; // ID here to allow for multiple loaders.

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
        std::vector<Image_Load_Request> requests;
        std::vector<Image>              images;
        std::mutex mtx;

        void loader_thread_fn() {
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
                    images[image.id] = image;
                }
            }
        }

    public:
        Image_Loader() {
            requests.reserve(16);
            images.reserve(32);
        }

        void start() {
            std::thread t([this] { loader_thread_fn(); });
            t.detach();
        }

        int add_request(const char* file_name) {
            Image_Load_Request request(file_name);
            std::lock_guard<std::mutex> lock(mtx);
            requests.push_back(request);

            return request.image_id;
        }
    };


    // @temporary: dirty singleton ...
    Image_Loader* get_loader() {
        static Image_Loader* loader = nullptr;
        if (loader == nullptr) {
            loader = new Image_Loader();
            loader->start();
        }

        return loader;
    }


    Image
    load_image(const char *file_name) {
        Image i;
        i.id = get_loader()->add_request(file_name);

        return i;
    }

}
