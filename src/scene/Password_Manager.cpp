
// Implements:
#include <scene/Password_Manager.hpp>

// Dependencies:
#include <sphinx/sphinx.hpp>
#include <sphinx/Image.hpp>

#include <filesystem>
namespace fs = std::filesystem;

#include <imgui.h>
#include <imgui_internal.h>
#include <file_dialog/ImGuiFileDialog.h>

namespace im = ImGui_Extended;

namespace sphinx {

    /*
        Helper function to draw an image to the screen.
    */
    static int minx(int a, int b) {
        return a < b ? a : b;
    }

    static void
    draw_image(const std::string& name, ImVec2 max_dimensions = {256, 256}) {
        const Image& image = get_image(name);
        if (image.is_ready_to_render()) {
        #pragma warning(push)
        #pragma warning(disable : 4312) // Typecast from: (unsigned int) 32bit to (void*) 64bit
            ImTextureID tex_id = (ImTextureID)image.texture_id;

            /*
                Calculate image size to fit within { max_dimensions } while retaining aspect ratio.
            */
            ImVec2 size = { (f32)image.width, (f32)image.height };
            ImVec2 uv_min = { 0.0f, 0.0f };
            ImVec2 uv_max = { 1.0f, 1.0f };

            // Compute aspect ratio and adjust size
            float aspect_ratio = size.x / size.y;
            if (size.x > max_dimensions.x || size.y > max_dimensions.y) {
                if (aspect_ratio > 1.0f) {
                    size.x = max_dimensions.x;
                    size.y = max_dimensions.x / aspect_ratio;
                } else {
                    size.y = max_dimensions.y;
                    size.x = max_dimensions.y * aspect_ratio;
                }
            }

            ImGui::Image(tex_id, size, uv_min, uv_max);
        #pragma warning(pop)
        } else {
            ImDrawList* draw_list = ImGui::GetCurrentWindow()->DrawList;
            ImVec2 pos            = { ImGui::GetCursorPosX(), ImGui::GetCursorPosY() };
            f64 time              = ImGui::GetTime();
            u8 blue               = max(50, (int)(255.0f * std::cos(time)));

            draw_list->AddRectFilled(
                pos,
                { pos.x + max_dimensions.x, pos.y + max_dimensions.y },
                IM_COL32(50, 50, blue, 255)
            );
        }
    }


    static void
    image_store_msg(const std::string& name, char* master_key_buffer, char* content_buffer) {
        Image& image = get_image(name);

        if (image.is_ready_to_write()) {
            // Temporary: encrypt
            AES128 aes(master_key_buffer);
            AES_String content = content_buffer;
            AES_String encrypted = aes.encrypt(content);

            // Just test it out quickly!
            u8* encrypted_content_data   = encrypted.blocks[0].data;
            int encrypted_content_length = encrypted.size_in_bytes();

            image.try_write(Image_Message(encrypted_content_length, encrypted_content_data), name);
        }
    }

    static void
    image_load_msg(const std::string& name, char* master_key_buffer, char* content_buffer) {
        Image& image = get_image(name);

        if (image.is_ready_to_write()) {
            Image_Message msg(0, nullptr);
            image.try_read(msg);

            // Temporary: decrypt:
            AES128 aes(master_key_buffer);
            AES_String encrypted_content((char*)msg.data);
            AES_String decrypted = aes.decrypt(encrypted_content);
        }
    }

    /*
        We can add a magic header of sorts to make sure a thing works.
    */
    struct Image_Data {
        std::string image_name;
        std::string description;
    };


    /*
        Get the executable path, since C++ is somehow unable to provide this.
    */
    static inline std::string
    get_executable_dir() {
        // Get the full executable path which includes executable name:
        char path[MAX_PATH] = {0};
        GetModuleFileNameA(NULL, path, MAX_PATH);

        // Terminate the path to exclude executable name:
        int length      = cstr_length(path);
        int name_length = cstr_length(PROJECT_NAME);

        EXPECT(length - (name_length + 4) >= 0);
        path[length - (name_length + 4)] = '\0';

        // Executable directory:
        return path;
    }

    static inline void
    to_disk(const std::string& path, const std::string& data) {
        std::ofstream out_file(path, std::ios::binary);
        EXPECT(out_file);

        out_file.write(data.c_str(), data.size());
        out_file.close();
    }

    static inline std::string
    from_disk(const std::string& path) {
        std::ifstream in_file(path, std::ios::binary);
        EXPECT(in_file);

        std::ostringstream buffer;
        buffer << in_file.rdbuf();

        return buffer.str();
    }

    constexpr const char*
    KEY_VERIFICATION_STRING = "When the terrain disagrees with the map, trust the terrain.";

    enum class Scene_Status {
        FIRST    = 0,
        LOGIN    = 1,
        MAIN     = 2,
        SHUTDOWN = 4
    };

    /*
        @todo: cleanup everything, split out the scene into smaller ones?
    */
    struct Scene_Context {
        std::vector<std::string> images;
        std::vector<std::string> errors;
        AES128*                  aes; // temp, before we create the encrypt thread...
        Scene_Status             scene_status;
        bool                     hide_key_input_text;
        bool                     is_authenticated;
        char                     key_input_buffer[BLOCK_SIZE];
        char                     repeat_key_buffer[BLOCK_SIZE];
        char                     content_input_buffer[BLOCK_SIZE*4];
        fs::path                 state_file_path;
        fs::path                 state_key_path;

        Scene_Context():
            key_input_buffer{},
            content_input_buffer{},
            repeat_key_buffer{},
            aes(nullptr)
        {
            images.reserve(32);

            state_file_path = get_executable_dir();
            state_file_path /= ".sphinx_state/data.bin";

            state_key_path = get_executable_dir();
            state_key_path /= ".sphinx_state/key.bin";

            if (fs::exists(state_key_path)) {
                scene_status = Scene_Status::LOGIN;
            } else {
                scene_status = Scene_Status::FIRST;
            }
        }

        bool
        state_exists_on_disk() {
            return fs::exists(state_file_path);
        }

        bool
        is_key_valid() {
            EXPECT(aes != nullptr);

            // Read out the encoded secret:
            const std::string secret = from_disk(state_key_path.string());

            // Decode the secret:
            std::string str = aes->decrypt(secret.c_str()).to_string();
            if (str.compare(KEY_VERIFICATION_STRING) == 0) {
                std::cout << "The key is valid!" << std::endl;
                return true;
            }

            std::cout << str << std::endl;
            return false;
        }

        void
        try_create_key() {
            int master_key_length = cstr_length(key_input_buffer);

            if (master_key_length < 8) {
                errors.push_back("Master Key must be at least 8 characters long!");
                return;
            }

            int repeat_key_length = cstr_length(repeat_key_buffer);

            if (master_key_length != repeat_key_length) {
                errors.push_back("Provided keys are not the same!");
                return;
            }

            for (int i = 0; i < master_key_length; ++i) {
                if (key_input_buffer[i] != repeat_key_buffer[i]) {
                    errors.push_back("Provided keys are not the same!");
                    return;
                }
            }

            // Generate the key:
            aes = new AES128(key_input_buffer);

            EXPECT(aes);
            EXPECT(!fs::exists(state_key_path));

            fs::create_directories(state_key_path.parent_path());

            // Write the key to disk:
            std::string encrypted_secret = aes->encrypt(KEY_VERIFICATION_STRING).to_string();
            to_disk(state_key_path.string(), encrypted_secret);

            // Temporary bypass the authentication process:
            scene_status = Scene_Status::MAIN;
            errors.clear();
        }

        void
        try_login() {
            int master_key_length = cstr_length(key_input_buffer);
            if (master_key_length < 8) {
                errors.push_back("Invalid Key, must be 8 or more characters!");
                return;
            }

            aes = new AES128(key_input_buffer);

            if (is_key_valid()) {
                scene_status = Scene_Status::MAIN;
                errors.clear();
            }
        }

        void
        write_to_disk() {
            /*
                Write Sphinx data to disk, all we care about are images and the master key.
                We expect this to be relative to the executable: ./sphinx/state.bin
            */
            if (!state_exists_on_disk()) {
                fs::create_directories(state_file_path.parent_path());
                std::cout << state_exists_on_disk() << std::endl;
                std::string data_mock = "";

                to_disk(state_file_path.string(), data_mock);
            } else {
                std::string data = from_disk(state_file_path.string());
                std::cout << data << std::endl;
            }
        }

        void
        load_from_disk() {
            EXPECT(state_exists_on_disk());
        }
    };

    Password_Manager::Password_Manager()
    : context(std::make_unique<Scene_Context>()) {
    }

    Password_Manager::~Password_Manager() {}

    void
    Password_Manager::init() {
        im::set_window_flags(
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoMove     |
            ImGuiWindowFlags_NoResize   |
            ImGuiWindowFlags_NoTitleBar
        );

        int dockspace_id = im::main_dockspace("##dockspace");
        im::window("Login");
        im::window("Main");
    }

    bool
    Password_Manager::run(f64 delta_time) {
        int dockspace_id = im::main_dockspace("##dockspace");

        if (!context->errors.empty()) {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, {.6f, .2f, .2f, 1.0f});
            if (im::window("##errors")) {
                ImGui::PushStyleColor(ImGuiCol_Text, {1.0f, 1.0f, 1.0f, 1.0f});
                for (const std::string& error : context->errors) {
                    ImGui::Text("%s", error.c_str());
                }
                ImGui::PopStyleColor();
                if (ImGui::Button("Dismiss")) {
                    context->errors.clear();
                }
            }
            ImGui::PopStyleColor();
        }

        // If we are not authenticated:
        switch (context->scene_status) {
        /*
            When we first open the application or screw the files up:
        */
        case Scene_Status::FIRST:
            if (im::window("##first")) {
                ImGui::Text("==First==");
                ImGui::Checkbox("Show password", &context->hide_key_input_text);
                ImGui::SetNextItemWidth(BLOCK_SIZE*8);
                ImGui::InputText(
                    "##master_key_input",
                    context->key_input_buffer,
                    BLOCK_SIZE,
                    context->hide_key_input_text ? ImGuiInputTextFlags_Password : ImGuiInputTextFlags_None
                );
                ImGui::SetNextItemWidth(BLOCK_SIZE*8);
                ImGui::InputText(
                    "##repeat_key_input",
                    context->repeat_key_buffer,
                    BLOCK_SIZE,
                    context->hide_key_input_text ? ImGuiInputTextFlags_Password : ImGuiInputTextFlags_None
                );

                if (ImGui::Button("Confirm")) {
                    context->try_create_key();
                }

                im::dock_to_center("##first", dockspace_id);
            }
            break;
        /*
            Login screen:
        */
        case Scene_Status::LOGIN:
            if (im::window("##login")) {
                ImGui::Text("==Login==");
                ImGui::Checkbox("Show password", &context->hide_key_input_text);
                ImGui::SetNextItemWidth(BLOCK_SIZE*8);
                ImGui::InputText(
                    "##master_key_input",
                    context->key_input_buffer,
                    BLOCK_SIZE,
                    context->hide_key_input_text ? ImGuiInputTextFlags_Password : ImGuiInputTextFlags_None
                );
                if (ImGui::Button("Login")) {
                    context->try_login();
                }
                im::dock_to_center("##login", dockspace_id);
            }
            break;
        case Scene_Status::MAIN:
            if (im::window("##main")) {
                ImGui::Text("==Main==");
                ImGui::Text("Welcome!");
            }
            break;
        case Scene_Status::SHUTDOWN:
            break;
        }

        return true;
    }

    void
    Password_Manager::cleanup() {

    }

}
