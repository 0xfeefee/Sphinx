
#include <scene/Password_Manager.hpp>
#include <sphinx/Image_Manager.hpp>
#include <util/Thread_Pool.hpp>


int main() {
    using namespace sphinx;

    Application app(std::make_unique<Password_Manager>());
    app.run();

    return 0;
}

