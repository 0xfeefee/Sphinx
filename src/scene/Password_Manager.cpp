
// Implements:
#include <scene/Password_Manager.hpp>

// Dependencies:
#include <sphinx/sphinx.hpp>

#include <imgui.h>
#include <imgui_internal.h>
namespace im = ImGui_Extended;

namespace sphinx {

    void
    Password_Manager::init() {
        im::set_window_flags(
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoTitleBar
        );

        int dockspace_id = im::main_dockspace("##dockspace");
        im::window("Main");

        // Dock to the center:
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderDockWindow("Main", dockspace_id);
        ImGui::DockBuilderFinish(dockspace_id);
    }

    bool
    Password_Manager::run(f64 delta_time) {
        im::main_dockspace("##dockspace");
        if (im::window("Main")) {
            ImGui::Text("Hello!");
        }

        return true;
    }

    void
    Password_Manager::cleanup() {

    }

}
