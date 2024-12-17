
#pragma once

namespace sphinx {

    void
    run_platform_window(Unique<Base_Scene> scene);

    int
    main_dockspace(const char* name);

    bool
    window(const char* title, bool* is_open = nullptr);


} // sphinx
