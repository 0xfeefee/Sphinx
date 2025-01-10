
#pragma once

namespace sphinx {

    /*
        User implements the scene.
    */
    class Base_Scene {
    public:
        Base_Scene()          = default;
        virtual ~Base_Scene() = default;

        virtual void
        init() = 0;

        virtual bool
        run(f64 delta_time) = 0;

        virtual void
        cleanup() = 0;
    };

    /*
        Applications runs the user scene.
    */
    struct UI_Context;
    class Application final {
    private:
        Unique<Base_Scene> scene;
        Unique<UI_Context> context;

    public:
        Application(Unique<Base_Scene> scene);
        ~Application();

        void
        run();
    };

}
