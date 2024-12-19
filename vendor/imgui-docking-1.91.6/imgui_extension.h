
#pragma once

namespace ImGui_Extended {

	/*
		UI elements we care about:
	*/
    int
    main_dockspace(const char* name);

    void
    set_window_flags(ImGuiWindowFlags flags);

    bool
    window(const char* title, bool* is_open = nullptr);

    void
    dock_to_center(const char* window, ImGuiID dock_node_id);

    /*
    	Internals, do not use directly!
    */
	void
	initialize_context();

	void
	destroy_context();

    void
    close_last_window();

}