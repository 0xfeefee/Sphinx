
#pragma once

namespace sphinx {

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

    struct Context;

    class Application {
    private:
        Unique<Base_Scene> scene;
        Unique<Context>    context;

    public:
        Application(Unique<Base_Scene> scene);
        virtual ~Application() = default;

        void
        run();
    };

}
