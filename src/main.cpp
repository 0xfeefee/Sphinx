
#include <sphinx/AES.hpp>
#include <sphinx/gui.hpp>

#include <imgui.h>

namespace sphinx {
    class Main_Scene final : public Base_Scene {
    public:
        Main_Scene() {}
        ~Main_Scene() {}

        void
        init() {
            ImGui::StyleColorsClassic();
        }

        bool
        run(f64 delta_time) {
            return window("Hello, World!");
        }

        void
        cleanup() {
            std::cout << "Cleanup!\n";
        }
    };
}

int main() {
    using namespace sphinx;

    // Initialize AES context
    AES128 aes("Harpocrates");
    AES_String text(
        R"(The world is indeed full of peril, and in it there are many dark places;
        but still there is much that is fair, and though in all lands love is now mingled with grief,
        it grows perhaps the greater.)"
    );

    AES_String cipher = aes.encrypt(text);
    AES_String output = aes.decrypt(cipher);

    std::cout << "Plain text: ";
    text.print();

    std::cout << "Cipher text: ";
    cipher.print();

    std::cout << "Output text: ";
    output.print();

    Unique<Main_Scene> main_scene = std::make_unique<Main_Scene>();
    run_platform_window(std::move(main_scene));

    return 0;
}

