#ifndef GLFW_INTERACTION_H
#define GLFW_INTERACTION_H

#define GLM_FORCE_RADIANS
#include <glm\glm.hpp>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

const float pi = 3.14159265358979323846264338327950288;
const float dAngle = pi / 60;
const float dr = 0.5f;

const float t0 = 0.0f;
const float phi0 = pi / 2;

struct CameraEyeLookAt {
	glm::vec3 eye = {0,0,5};
	glm::vec3 center = {0,0,0};
	uint8_t upAxis = 0; // unused
	// Cached floats
	float t1 = -pi/2;
	float phi1 = pi/2;
	float cphi1 = cos(phi1);
	float sphi1 = sin(phi1);
	float ct1 = cos(t1);
	float st1 = sin(t1);
	bool bottom = false;
	bool top = false;
	bool keyCallbackMoveCameraEye(int key, int scancode, int action, int mods);
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

#endif

