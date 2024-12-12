
// Implements:
#include <sphinx/gui.hpp>

// Dependencies (3rd_party):
#define GLFW_INCLUDE_NONE
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


namespace sphinx {


    void
    run_platform_window() {
        static GLFWwindow* h_window = NULL;

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

        h_window = glfwCreateWindow(1920, 1080, "Sphinx", NULL, NULL);
        if (!h_window) {
            ERROR_IF(true, "Failed to create a GLFW platform window!");
            return;
        }

        glfwMakeContextCurrent(h_window);
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

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(h_window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        glViewport(0, 0, 1920, 1080);

        while (!glfwWindowShouldClose(h_window)) {
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Frame:
            {
                ImGui::ShowDemoWindow();
                if (glfwGetKey(h_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                    glfwSetWindowShouldClose(h_window, GLFW_TRUE);
                }
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(h_window);
            glfwPollEvents();
        }
    }

}