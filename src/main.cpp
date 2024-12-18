
#include <scene/Password_Manager.hpp>

int main() {
    using namespace sphinx;

    Application app(std::make_unique<Password_Manager>());
    app.run();

    return 0;
}

