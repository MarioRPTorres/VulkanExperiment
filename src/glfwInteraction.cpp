#include "glfwInteraction.h"


bool CameraEyeLookAt::keyCallbackMoveCameraEye(int key, int scancode, int action, int mods) {
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

			center[0] = eye[0] + ct1 * sphi1;
			center[2] = eye[2] + st1 * sphi1;
			center[1] = eye[1] - cphi1;

			if (bottom) bottom = false;

			return true;
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

			center[0] = eye[0] + ct1 * sphi1;
			center[2] = eye[2] + st1 * sphi1;
			center[1] = eye[1] - cphi1;

			if (top) top = false;
			return true;
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

		center[0] = eye[0] + sphi1 * ct1;
		center[2] = eye[2] + sphi1 * st1;
		return true;

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

		center[0] = eye[0] + sphi1 * ct1;
		center[2] = eye[2] + sphi1 * st1;
		return true;
	}
	// Walk camera forward
	else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		center[0] += dr;
		eye[0] += dr;
		return true;
	}
	// Walk camera backwar
	else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		center[0] -= dr;
		eye[0] -= dr;
		return true;
	}
	// Walk camera forward
	else if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		center[2] += dr;
		eye[2] += dr;
		return true;
	}
	// Walk camera backward
	else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		center[2] -= dr;
		eye[2] -= dr;
		return true;
	}
	// Walk camera forward
	else if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		center[1] += dr;
		eye[1] += dr;
		return true;
	}
	// Walk camera backward
	else if (key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		center[1] -= dr;
		eye[1] -= dr;
		return true;
	}
	return false;
}


float r = 200.0f;
float cameraEye[3] = { r * cos(t0) * sin(phi0) , r * cos(phi0), -r * sin(t0) * sin(phi0) };

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


