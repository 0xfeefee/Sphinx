
#pragma once

namespace sphinx {

    class UI_Root {
    public:
        virtual void
        run();
    };

    void
    run_platform_window();

    int
    main_dockspace(const char* name);

    bool
    window(const char* title, bool* is_open = nullptr);


} // sphinx
