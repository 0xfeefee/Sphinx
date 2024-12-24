
#pragma once
#include <sphinx/GUI.hpp>

namespace sphinx {

    struct Scene_Context;

    class Password_Manager final : public Base_Scene {
    private:
        Unique<Scene_Context> context;

    public:
        Password_Manager();
        ~Password_Manager();

        void
        init();

        bool
        run(f64 delta_time);

        void
        cleanup();
    };

}
