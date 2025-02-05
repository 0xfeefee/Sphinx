
#pragma once
#include <sphinx/GUI.hpp>

namespace sphinx {

    struct Scene_Context;

    class Password_Manager_Scene final : public Base_Scene {
    private:
        Unique<Scene_Context> context;

    public:
        Password_Manager_Scene();
        ~Password_Manager_Scene();

        void
        init();

        bool
        run(f64 delta_time);

        void
        cleanup();
    };

}
