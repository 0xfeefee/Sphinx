
// Implements:
#include <sphinx/sphinx.hpp>

// Dependencies (3rd_party):
#define GLFW_INCLUDE_NONE
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
namespace im = ImGui_Extended;

#include <file_dialog/ImGuiFileDialog.h>


namespace sphinx {

    struct UI_Context {
        GLFWwindow* h_window;
        bool        is_first_frame;

        UI_Context()
        : h_window(NULL),
          is_first_frame(true) {}

        ~UI_Context() = default;
    };


    Application::Application(Unique<Base_Scene> scene)
    : scene(std::move(scene)),
      context(std::make_unique<UI_Context>()) {}

    Application::~Application() {}

    void
    Application::run() {
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

        context->h_window = glfwCreateWindow(1920, 1080, "Sphinx", NULL, NULL);
        if (!context->h_window) {
            ERROR_IF(true, "Failed to create a GLFW platform window!");
            return;
        }

        glfwMakeContextCurrent(context->h_window);
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
        io.IniFilename = nullptr;

        ImGui_ImplGlfw_InitForOpenGL(context->h_window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        glViewport(0, 0, 1920, 1080);

        // Main loop:
        f64 last_frame_time = glfwGetTime();
        while (!glfwWindowShouldClose(context->h_window)) {
            // Calculate delta:
            f64 current_frame_time = glfwGetTime();
            f64 delta_time = current_frame_time - last_frame_time;
            last_frame_time = current_frame_time;

            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (!context->is_first_frame) {
                if (!scene->run(delta_time) || glfwGetKey(context->h_window, GLFW_KEY_ESCAPE)) {
                    glfwSetWindowShouldClose(context->h_window, GLFW_TRUE);
                }
            } else {
                scene->init();
                context->is_first_frame = false;
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(context->h_window);
            glfwPollEvents();
        }

        // Not necessary since we exit the application here ....
        // ImGui::DestroyContext();

        scene->cleanup();
    }

} // sphinx
