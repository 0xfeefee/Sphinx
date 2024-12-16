
// Implements:
#include <sphinx/gui.hpp>

// Dependencies (3rd_party):
#define GLFW_INCLUDE_NONE
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace im = ImGui;

namespace sphinx {

    struct Context {
        GLFWwindow* h_window   = NULL;
        bool any_window_opened = false;
        ImGuiWindowFlags default_window_flags = ImGuiWindowFlags_NoCollapse;
    };

    static Context* ctx = nullptr;

    static inline void
    close_any_opened_window() {
        if (ctx->any_window_opened == true) {
            im::End();
            ctx->any_window_opened = false;
        }
    }

    void
    run_platform_window() {
        ctx = new Context();

        /*
        ## Init: GLFW

            Initialize GLFW, create the platform window and OpenGL context.
        */
        if (!glfwInit()) {
            ERROR_IF(true, "Failed to initialize GLFW!");
            return;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        ctx->h_window = glfwCreateWindow(1920, 1080, "Sphinx", NULL, NULL);
        if (!ctx->h_window) {
            ERROR_IF(true, "Failed to create a GLFW platform window!");
            return;
        }

        glfwMakeContextCurrent(ctx->h_window);
        if (gl3wInit() != 0) {
            ERROR_IF(true, "Failed to initialize gl3w!");
            return;
        }

        glViewport(0, 0, 1920, 1080);
        glfwSwapInterval(1);
        glClearColor(0.2f, 0.3f, 0.3f, 0.0f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        /*
        ## Init: ImGui
        */
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(ctx->h_window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        glViewport(0, 0, 1920, 1080);

        while (!glfwWindowShouldClose(ctx->h_window)) {
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Frame:
            {
                main_dockspace("##Main");

                window("Test");
                window("Hello");

                ImGui::ShowDemoWindow();
                if (glfwGetKey(ctx->h_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                    glfwSetWindowShouldClose(ctx->h_window, GLFW_TRUE);
                }

                close_any_opened_window();
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(ctx->h_window);
            glfwPollEvents();
        }
    }

    int
    main_dockspace(const char* dockspace_window_name) {
        static ImGuiViewport* viewport = im::GetMainViewport();
        ERROR_IF(viewport == NULL);

        // Set the size and position of the new window:
        im::SetNextWindowViewport(viewport->ID);
        im::SetNextWindowSize(viewport->WorkSize);
        im::SetNextWindowPos({0,0});

        // Push the custom background/padding style:
        im::PushStyleColor(ImGuiCol_WindowBg, {0,0,0,255});

        // Dock-space window: We also give it a custom flag identifier
        ImGuiWindowFlags dockspace_window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                                  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                                                  ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGuiID dockspace_id = 0;

        // Run the dock-space window:
        im::Begin(dockspace_window_name, NULL, dockspace_window_flags); {
            im::PopStyleColor();

            // Run the dockspace using main window as host:
            ImGuiID dockspace_window_id = im::GetID(dockspace_window_name);
            dockspace_id = im::DockSpace(dockspace_window_id, {0.0f, 0.0f}, ImGuiDockNodeFlags_None);

            im::End();
        }

        return dockspace_id;
    }

    bool
    window(const char* title, bool* is_open) {
        close_any_opened_window();

        bool window = false;
        if (is_open == NULL || (is_open && *is_open)) {
            window = im::Begin(title, is_open, ctx->default_window_flags);
            ctx->any_window_opened = true;
        }

        return window;
    }

}

