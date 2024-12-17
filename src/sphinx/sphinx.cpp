
// Implements:
#include <sphinx/sphinx.hpp>

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
        GLFWwindow*         h_window;
        bool                last_window_open;
        ImGuiWindowFlags    default_window_flags;
        Unique<Base_Scene>  scene;

        Context() :
            h_window(NULL),
            last_window_open(false),
            default_window_flags(ImGuiWindowFlags_NoCollapse),
            scene(nullptr) {
        }
    };

    static inline void
    close_last_window(Unique<Context>& ctx) {
        if (ctx->last_window_open) {
            im::End();
            ctx->last_window_open = false;
        }
    }

    Application::Application(Unique<Base_Scene> scene)
    : scene(std::move(scene)), context(std::make_unique(Context))
    {}

    void
    Application::run() {
        ctx->scene = std::move(scene);

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

        ImGui_ImplGlfw_InitForOpenGL(ctx->h_window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        glViewport(0, 0, 1920, 1080);

        ctx->scene->init();

        while (!glfwWindowShouldClose(ctx->h_window)) {
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Frame:
            {
                main_dockspace("##Main");
                ctx->scene->run(0.0f);

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

        ctx->scene->cleanup();
    }

}


namespace sphinx {

    static inline void
    close_any_opened_window() {
        if (ctx->any_window_opened == true) {
            im::End();
            ctx->any_window_opened = false;
        }
    }

    void
    run_platform_window(Unique<Base_Scene> scene) {
        ctx = new Context();
        ctx->scene = std::move(scene);

        // scene->init();

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

        ImGui_ImplGlfw_InitForOpenGL(ctx->h_window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        glViewport(0, 0, 1920, 1080);

        ctx->scene->init();

        while (!glfwWindowShouldClose(ctx->h_window)) {
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Frame:
            {
                main_dockspace("##Main");
                ctx->scene->run(0.0f);

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

        ctx->scene->cleanup();
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

}

