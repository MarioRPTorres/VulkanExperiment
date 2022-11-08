#include "glfwInteraction.h"

const float pi = 3.14159265358979323846264338327950288;
const float dAngle = pi / 60;
const float dr = 0.5f;

float r = 20.0f;
const float t0 = 0.0f;
const float phi0 = pi / 2;

glm::vec3 cameraEye = { r * cos(t0) * sin(phi0) , r * cos(phi0), -r * sin(t0) * sin(phi0) };

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static float  t1 = t0, phi1 = phi0;
	static float cphi1 = cos(phi1), sphi1 = sin(phi1);
	static float ct1 = cos(t1), st1 = sin(t1);
	static float rct1 = r * ct1, rst1 = r * st1;
	static float rsphi1 = r * sphi1;


	static bool bottom = false;
	static bool top = false;
	static bool near = false;

	if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		if (!top) {
			if ((phi1 - dAngle) < 0.01f) {
				top = true;
				phi1 = 0.01f;
			}
			else {
				phi1 -= dAngle;
			}

			cphi1 = cos(phi1);
			sphi1 = sin(phi1);

			cameraEye[0] = rct1 * sphi1;
			cameraEye[2] = rst1 * sphi1;
			cameraEye[1] = -r * cphi1;

			rsphi1 = r * sphi1;

			if (bottom) bottom = false;
		}
	}
	else if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		if (!bottom) {

			if ((phi1 + dAngle) > pi) {
				bottom = true;
				phi1 = pi - 0.01f;
			}
			else {
				phi1 += dAngle;
			}

			cphi1 = cos(phi1);
			sphi1 = sin(phi1);

			cameraEye[0] = rct1 * sphi1;
			cameraEye[2] = rst1 * sphi1;
			cameraEye[1] = -r * cphi1;

			if (top) top = false;
		}
	}
	else if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {

		if ((t1 + dAngle) > pi) {
			t1 = -pi;
		}
		else {
			t1 += dAngle;
		}

		ct1 = cos(t1);
		st1 = sin(t1);
		rct1 = r * ct1;
		rst1 = r * st1;
		cameraEye[0] = sphi1 * rct1;
		cameraEye[2] = sphi1 * rst1;

	}
	else if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {

		if ((t1 - dAngle) < -pi) {
			t1 = pi;
		}
		else {
			t1 -= dAngle;
		}

		ct1 = cos(t1);
		st1 = sin(t1);
		rct1 = r * ct1;
		rst1 = r * st1;
		cameraEye[0] = sphi1 * rct1;
		cameraEye[2] = sphi1 * rst1;
	}
	// Pull camera forward
	else if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		if (!near) {
			if ((r - dr) < 0) {
				near = true;
				r = 0.5f;
			}
			else {
				r -= dr;
			}

			rct1 = r * ct1;
			rst1 = r * st1;

			cameraEye[0] = rct1 * sphi1;
			cameraEye[2] = rst1 * sphi1;
			cameraEye[1] = -r * cphi1;
		}
	}
	// Pull camera backward
	else if (key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT)) {

		r += dr;

		rct1 = r * ct1;
		rst1 = r * st1;

		cameraEye[0] = rct1 * sphi1;
		cameraEye[2] = rst1 * sphi1;
		cameraEye[1] = -r * cphi1;

		if (near) near = false;
	}

	//printf("Theta: %f, Phi: %f, r: %f ; ", t1, phi1, r);
	//printf("Camera Eye: %f , %f , %f\n", cameraEye[0], cameraEye[1], cameraEye[2]);
}


