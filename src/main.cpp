
#include <scene/Password_Manager.hpp>
#include <sphinx/Image_Manager.hpp>
#include <util/Thread_Pool.hpp>

using namespace sphinx;

int main() {

    Application app(std::make_unique<Password_Manager>());
    app.run();

    return 0;
}

