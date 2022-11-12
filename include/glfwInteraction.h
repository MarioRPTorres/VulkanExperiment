#ifndef GLFW_INTERACTION_H
#define GLFW_INTERACTION_H

#include "vulkan_engine.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebufferResizeCallback(GLFWwindow* window, int width, int height);

#endif

