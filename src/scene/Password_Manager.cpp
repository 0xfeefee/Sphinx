
// Implements:
#include <scene/Password_Manager.hpp>

// Dependencies:
#include <sphinx/sphinx.hpp>
#include <sphinx/Image.hpp>

#include <imgui.h>
#include <imgui_internal.h>
namespace im = ImGui_Extended;


namespace sphinx {

    /*
        Helper function to draw an image to the screen.
    */
    static void
    draw_image(const std::string& name, ImVec2 max_dimensions = {0, 0}) {
        const Image& image = get_image(name);
        if (image.is_ready_to_render()) {
        #pragma warning(push)
        #pragma warning(disable : 4312) // Typecast from: (unsigned int) 32bit to (void*) 64bit
            ImTextureID tex_id = (ImTextureID)image.texture_id;

            /*
                Calculate image size and UV-s such that it fits within the { max_dimensions }.
            */
            ImVec2 size = { (f32)image.width, (f32)image.height };
            ImVec2 uv_min = { 0.0f, 0.0f };
            ImVec2 uv_max = { 1.0f, 1.0f };

            // Adjust size and UVs if the image exceeds max_dimensions
            if (max_dimensions.x > 0 && size.x > max_dimensions.x) {
                uv_max.x = max_dimensions.x / size.x; // Adjust UV to show only part of the width
                size.x = max_dimensions.x;
            }

            if (max_dimensions.y > 0 && size.y > max_dimensions.y) {
                uv_max.y = max_dimensions.y / size.y; // Adjust UV to show only part of the height
                size.y = max_dimensions.y;
            }

            ImGui::Image(tex_id, size, uv_min, uv_max);
        #pragma warning(pop)
        } else {
            ImDrawList* draw_list = ImGui::GetCurrentWindow()->DrawList;
            ImVec2 pos = { ImGui::GetCursorPosX(), ImGui::GetCursorPosY() };
            draw_list->AddRectFilledMultiColor(
                pos,
                { pos.x + max_dimensions.x, pos.y + max_dimensions.y },
                IM_COL32(50, 100, 150, 255),
                IM_COL32(150, 100, 150, 255),
                IM_COL32(100, 150, 50, 255),
                IM_COL32(255, 50, 100, 255)
            );
        }
    }

    static void
    image_store_msg(const std::string& name, char* master_key_buffer, char* content_buffer) {
        Image& image = get_image(name);

        if (image.is_ready_to_write()) {
            // Just test it out quickly!
            int len = cstr_length(master_key_buffer);
            image.try_write(Image_Message(len, (u8*)master_key_buffer), name);
        }
    }

    static void
    image_load_msg(const std::string& name, char* master_key_buffer, char* content_buffer) {
        Image& image = get_image(name);

        if (image.is_ready_to_write()) {
            Image_Message msg(0, nullptr);
            image.try_read(msg);

            printf("Message: (%s)\n", msg.data);
        }
    }

    int minx(int a, int b) {
        return a < b ? a : b;
    }

    void
    show_grid_layout(int columns, ImVec2 itemSize, int itemCount) {
        float contentWidth = ImGui::GetContentRegionAvail().x;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        int itemsPerRow = minx(columns, static_cast<int>((contentWidth + spacing) / (itemSize.x + spacing)));
        float totalWidth = itemsPerRow * itemSize.x + (itemsPerRow - 1) * spacing;
        float offsetX = (contentWidth - totalWidth) / 2.0f;

        if (offsetX > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

        for (int i = 0; i < itemCount; i++) {
            if (i > 0 && i % itemsPerRow == 0) {
                ImGui::NewLine();
                if (offsetX > 0.0f) {
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
                }
            }

            ImGui::PushID(i);
            ImGui::Button("Item", itemSize);
            ImGui::PopID();

            if ((i + 1) % itemsPerRow != 0) {
                ImGui::SameLine();
            }
        }
    }

    /*
        We can add a magic header of sorts to make sure a thing works.
    */
    struct Image_Data {
        std::string image_name;
        std::string descriptionj;
    };

    struct Scene_Context {
        AES_User_Key             user_key;
        std::vector<std::string> images;
        bool                     hide_key_input_text;
        char                     key_input_buffer[BLOCK_SIZE];
        char                     content_input_buffer[BLOCK_SIZE*4];

        Scene_Context(): key_input_buffer{}, content_input_buffer{} {
            images.reserve(32);
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
        im::window("Main");
        im::dock_to_center("Main", dockspace_id);

        for (int i = 0; i < 32; ++i) {
            context->images.push_back("bin/Iceland.png");
        }
    }

    bool
    Password_Manager::run(f64 delta_time) {
        im::main_dockspace("##dockspace");
        if (im::window("Main")) {

            // Main input:
            {
                ImGui::Separator();
                ImGui::Checkbox("Show password", &context->hide_key_input_text);
                ImGui::SetNextItemWidth(BLOCK_SIZE*8);
                ImGui::InputText(
                    "##master_key_input",
                    context->key_input_buffer,
                    BLOCK_SIZE,
                    context->hide_key_input_text ? ImGuiInputTextFlags_Password : ImGuiInputTextFlags_None
                );
                ImGui::SameLine();
                ImGui::InputText(
                    "##content_input",
                    context->content_input_buffer,
                    BLOCK_SIZE*4
                );
                if (ImGui::Button("Store")) {
                    image_store_msg(
                        "bin/Iceland.png",
                        context->key_input_buffer,
                        context->content_input_buffer
                    );
                }
                if (ImGui::Button("Load")) {
                    image_load_msg(
                        "bin/Iceland.png",
                        context->key_input_buffer,
                        context->content_input_buffer
                    );
                }
                ImGui::Separator();
            }

            static ImVec2 image_size = { 200, 200 };
            f32 container_width = ImGui::GetContentRegionAvail().x;
            f32 current_width = 0.0f;

            ImGui::NewLine();

            for (const auto& image_name: context->images) {
                if (current_width > 0.0f) {
                    if (current_width < container_width) {
                        ImGui::SameLine();
                    } else {
                        current_width = 0.0f;
                    }
                }

                draw_image(image_name, image_size);
                current_width += 210;
            }

            show_grid_layout(8, {300, 100}, 47);
        }

        return true;
    }

    void
    Password_Manager::cleanup() {

    }

}
