
#include <scene/Password_Manager_Scene.hpp>
#include <sphinx/Image_Manager.hpp>
#include <util/Thread_Pool.hpp>

using namespace sphinx;

int main() {

    Application app(std::make_unique<Password_Manager_Scene>());
    app.run();

    return 0;
}
