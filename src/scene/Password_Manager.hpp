
#pragma once
#include <sphinx/GUI.hpp>

namespace sphinx {

    struct Password_Manager_Context;

    class Password_Manager final : public Base_Scene {
    public:
        Password_Manager() {}
        ~Password_Manager() {}

        void
        init();

        bool
        run(f64 delta_time);

        void
        cleanup();
    };

}
