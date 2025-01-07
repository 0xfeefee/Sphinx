
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
    draw_image(const std::string& name, ImVec2 max_dimensions = { 256, 256 }) {
        const Image& image = get_image(name);
        if (image.is_ready_to_render()) {
            #pragma warning(push)
            #pragma warning(disable : 4312) // Typecast from: (unsigned int) 32bit to (void*) 64bit
            ImTextureID tex_id = (ImTextureID)image.texture_id;
            #pragma warning(pop)

            /*
                Calculate image size to fit within { max_dimensions } while retaining aspect ratio.
            */
            ImVec2 size = { (f32)image.width, (f32)image.height };
            ImVec2 uv_min = { 0.0f, 0.0f };
            ImVec2 uv_max = { 1.0f, 1.0f };

            if (max_dimensions.x <= 0.0f) max_dimensions.x = size.x;
            if (max_dimensions.y <= 0.0f) max_dimensions.y = size.y;

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
        } else {
            ImDrawList* draw_list = ImGui::GetCurrentWindow()->DrawList;
            ImVec2 pos            = { ImGui::GetCursorPosX(), ImGui::GetCursorPosY() };
            f64 time              = ImGui::GetTime();
            u8 blue               = max(50, (int)(255.0f * std::cos(time*4)));

            draw_list->AddRectFilled(
                pos,
                { pos.x + max_dimensions.x, pos.y + max_dimensions.y },
                IM_COL32(50, 50, blue, 255)
            );
        }
    }

    static bool
    draw_image_button(const std::string& name, ImVec2 max_dimensions = {256, 256}) {
        const Image& image = get_image(name);
        if (image.is_ready_to_render()) {
            #pragma warning(push)
            #pragma warning(disable : 4312) // Typecast from: (unsigned int) 32bit to (void*) 64bit
            ImTextureID tex_id = (ImTextureID)image.texture_id;
            #pragma warning(pop)

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

            if (ImGui::ImageButton(name.c_str(), tex_id, size, uv_min, uv_max)) {
                return true;
            }
        } else {
            ImDrawList* draw_list = ImGui::GetCurrentWindow()->DrawList;
            ImVec2 pos            = { ImGui::GetCursorPosX(), ImGui::GetCursorPosY() };
            f64 time              = ImGui::GetTime();
            u8 blue               = max(50, (int)(255.0f * std::cos(time*4)));

            draw_list->AddRectFilled(
                pos,
                { pos.x + max_dimensions.x, pos.y + max_dimensions.y },
                IM_COL32(50, 50, blue, 255)
            );
        }

        return false;
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
    KEY_VERIFICATION_STRING = "The most merciful thing in the world, I think, is the inability of the human mind to correlate all its contents. We live on a placid island of ignorance in the midst of black seas of infinity, and it was not meant that we should voyage far.";

    enum class Scene_Status {
        FIRST    = 0,
        LOGIN    = 1,
        MAIN     = 2
    };

    /*
        @todo: cleanup everything, split out the scene into smaller ones?
    */
    struct Scene_Context {
        std::string              selected_image;
        std::set<std::string>    images;
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
            std::string str = aes->decrypt(secret).to_string();
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
                load_from_disk();
            } else {
                errors.push_back("Invalid Key!");
            }
        }

        void
        write_images_to_disk() {
            std::ofstream out_file(state_file_path.string());
            EXPECT(out_file);

            for (const std::string& image_name : images) {
                out_file << image_name << ";";
            }

            out_file.close();
        }

        void
        load_from_disk() {
            if (!state_exists_on_disk()) {
                return;
            }

            std::string state = from_disk(state_file_path.string());
            size_t start      = 0;
            size_t end        = 0;

            while ((end = state.find(';', start)) != std::string::npos) {
                images.emplace(state.substr(start, end - start));
                start = end + 1;
            }
        }
    };

    Password_Manager::Password_Manager()
    : context(std::make_unique<Scene_Context>()) {}

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
                ImGui::Checkbox("Hide password", &context->hide_key_input_text);
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
                ImGui::Checkbox("Hide password", &context->hide_key_input_text);
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
        /*
            Image Grid, read and write passwords.
        */
        case Scene_Status::MAIN:
            if (im::window("##main")) {
                ImGui::Text("==Main==");
                ImGui::Text("Welcome!");

                // Add image button:
                if (ImGui::Button("Add")) {
                    IGFD::FileDialogConfig config;
                    config.path = ".";

                    ImGui::SetNextWindowSize({ 500, 500 });
                    ImGuiFileDialog::Instance()->OpenDialog(
                        "#image_picker",
                        "Choose file",
                        ".png",
                        config
                    );
                }
                ImGui::Separator();

                if (ImGuiFileDialog::Instance()->Display("#image_picker")) {
                    if (ImGuiFileDialog::Instance()->IsOk()) {
                        context->images.emplace(ImGuiFileDialog::Instance()->GetFilePathName());
                    }

                    ImGuiFileDialog::Instance()->Close();
                }

                // Draw the image grid:
                ImVec2 max_dimensions = { 256, 256 };
                ImVec2 dim = ImGui::GetContentRegionAvail();
                f32 cx = 0.0f;

                for (const std::string& image: context->images) {
                    if ((cx + max_dimensions.x) < dim.x -100.0f) {
                        ImGui::SameLine();
                        cx += max_dimensions.x;
                    } else {
                        cx = 0.0f;
                    }

                    if (draw_image_button(image, max_dimensions)) {
                        ImGui::OpenPopup("Message View##modal");
                        context->selected_image = image;
                    }
                }

                if (ImGui::BeginPopupModal("Message View##modal", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("%s", context->selected_image.c_str());
                    draw_image(context->selected_image);

                    ImGui::SetNextItemWidth(256.0f);
                    ImGui::InputText("##image_input_text", context->content_input_buffer, BLOCK_SIZE*4);
                    ImGui::SameLine();
                    if (ImGui::Button("Commit##modal")) {
                        Image& i = get_image(context->selected_image);
                    }
                    ImGui::PushStyleColor(ImGuiCol_Button, {0.4f, 0.1f, 0.1f, 1.0f});
                    if (ImGui::Button("Close##modal")) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::PopStyleColor();
                    ImGui::EndPopup();
                }

                im::dock_to_center("##main", dockspace_id);
            }
            break;
        }

        return true;
    }

    void
    Password_Manager::cleanup() {
        // Only write the images to disk if we are in the { MAIN } part of the scene.
        if (context->scene_status == Scene_Status::MAIN) {
            context->write_images_to_disk();
        }
    }

}
