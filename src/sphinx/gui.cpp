
// Implements:
#include <sphinx/gui.hpp>

#define GLFW_INCLUDE_NONE
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>


namespace sphinx {

    static GLFWwindow* h_window = NULL;

    void
    run_platform_window() {
        if (!glfwInit()) {
            return;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        h_window = glfwCreateWindow(1920, 1080, "Sphinx", NULL, NULL);
        if (!h_window) {
            return;
        }

        glfwMakeContextCurrent(h_window);
        if (gl3wInit() != 0) {
            return;
        }

        glViewport(0, 0, 1920, 1080);
        glfwSwapInterval(1);
        glClearColor(0.2f, 0.3f, 0.3f, 0.0f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        while (!glfwWindowShouldClose(h_window)) {
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(h_window);
            glfwPollEvents();
        }
    }

}