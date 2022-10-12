#include "main.h"

const float pi = 3.14159265358979323846264338327950288;
const float dAngle = pi / 60;

const float r = 3.5f;
const float t0 = 0.0f;
const float phi0 = pi / 2;

glm::vec3 cameraEye = { r*cos(t0)*sin(phi0) , r * cos(phi0), -r * sin(t0) * sin(phi0) };

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static float  t1 = t0, phi1 = phi0;
	static float sphi1 = sin(phi1);
	static float rct1 = r * cos( t1 ), rst1 = r * sin( t1 ) ;
	static float rsphi1 = r * sphi1;

	float ct2, st2, cphi2, sphi2;

	static bool bottom = false;
	static bool top = false;


	if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		if (!top) {
			if ((phi1 - dAngle) < 0.01f) {
				top = true;
				phi1 = 0.01f;
			}
			else {
				phi1 -= dAngle;
			}

			sphi1 = sin(phi1);

			cameraEye[0] = rct1 * sphi1;
			cameraEye[2] = rst1 * sphi1;
			cameraEye[1] = -r * cos(phi1);

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

			sphi1 = sin(phi1);

			cameraEye[0] = rct1 * sphi1;
			cameraEye[2] = rst1 * sphi1;
			cameraEye[1] = -r * cos(phi1);

			if (top) top = false;
		}
	}
	else if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)){
		
		printf("Pressed Right ");
		if ((t1 + dAngle) > pi) {
			t1 = -pi;
		}
		else {
			t1 += dAngle;
		}

		rct1 = r * cos(t1);
		rst1 = r * sin(t1);
		cameraEye[0] = sphi1 * rct1;
		cameraEye[2] = sphi1 * rst1;

	}
	else if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
		printf("Pressed Left ");
		if ((t1 - dAngle) < -pi) {
			t1 = pi;
		}
		else {
			t1 -= dAngle;
		}

		rct1 = r * cos(t1);
		rst1 = r * sin(t1);
		cameraEye[0] = sphi1 * rct1;
		cameraEye[2] = sphi1 * rst1;
	}

	printf("Theta: %f, Phi: %f", t1, phi1);
	printf("Camera Eye: %f , %f , %f\n", cameraEye[0], cameraEye[1], cameraEye[2]);
}



const glm::vec3 red = { 1.0f,0.0f,0.0f };
const glm::vec3 green = { 0.0f,1.0f,0.0f };
const glm::vec3 blue = { 0.0f,0.0f,1.0f };
const glm::vec3 yellow = { 1.0f,1.0f,0.0f };
const glm::vec3 white = { 1.0f,1.0f,1.0f };


struct botchedModel {
	std::vector<Vertex> v;
	std::vector<uint16_t> i;
	void (*preSPT) (std::vector<Vertex>&, std::vector<uint16_t>&);
};

void arrangePyramidVertex(std::vector<Vertex>& vert, std::vector<uint16_t>& ind) {
	const float preMat[16] = {
			1.0f, cosf(pi / 3.0f),  0.5f,													-1.0f,
			0.0f, sinf(pi / 3.0f),  tanf(pi / 6.0f) * 0.5f,									-0.577350269189626f,
			0.0f, 0.0f,				acosf(sqrtf(0.25f + pow(tanf(pi / 6.0f) * 0.5f, 2))),	-sqrtf(2 / 3.0f) * 2 / 4.0f,
			0.0f, 0.0f,				0.0f,													1.0f
	};
	const glm::mat4 spT = glm::transpose(glm::make_mat4(preMat));

	ind.resize(vert.size());
	for (int i = 0; i < vert.size(); i++) {
		vert.at(i).pos = glm::vec3(spT * glm::vec4(vert.at(i).pos, 1.0f));
		ind.at(i) = i;
	}
}

botchedModel pyramid = {
	{
	{ {0.0f, 0.0f, 0.0f}, red, {0.0f, 0.0f}},
	{ {1.0f, 0.0f, 0.0f}, red,{0.0f,1.0f}},
	{ {0.0f, 0.0f, 1.0f}, red,{1.0f,0.0f}},
	{ {0.0f, 0.0f, 1.0f}, white},
	{ {1.0f, 0.0f, 0.0f}, white},
	{ {0.0f, 1.0f, 0.0f}, white},
	{ {0.0f, 0.0f, 0.0f}, blue},
	{ {0.0f, 0.0f, 1.0f}, blue},
	{ {0.0f, 1.0f, 0.0f}, blue},
	{ {0.0f, 0.0f, 0.0f}, green},
	{ {0.0f, 1.0f, 0.0f}, green},
	{ {1.0f, 0.0f, 0.0f}, green},
	{ {1.0f, 0.0f, 0.0f}, red},
	{ {1.0f, 0.0f, 1.0f}, red},
	{ {0.0f, 0.0f, 1.0f}, red},
	{ {1.0f, 0.0f, 0.0f}, green},
	{ {2.0f, 0.0f, 0.0f}, green},
	{ {1.0f, 0.0f, 1.0f}, green},
	{ {0.0f, 0.0f, 1.0f}, blue},
	{ {1.0f, 0.0f, 1.0f}, blue},
	{ {0.0f, 0.0f, 2.0f}, blue},
	{ {0.0f, 0.0f, 1.0f}, blue},
	{ {0.0f, 1.0f, 1.0f}, blue},
	{ {0.0f, 1.0f, 0.0f}, blue},
	{ {0.0f, 1.0f, 0.0f}, yellow},
	{ {0.0f, 1.0f, 1.0f}, yellow},
	{ {0.0f, 2.0f, 0.0f}, yellow},
	{ {0.0f, 0.0f, 1.0f}, green},
	{ {0.0f, 0.0f, 2.0f}, green},
	{ {0.0f, 1.0f, 1.0f}, green},
	{ {1.0f, 0.0f, 1.0f}, yellow},
	{ {1.0f, 1.0f, 0.0f}, yellow},
	{ {0.0f, 1.0f, 1.0f}, yellow},
	{ {0.0f, 0.0f, 2.0f}, yellow},
	{ {1.0f, 0.0f, 1.0f}, yellow},
	{ {0.0f, 1.0f, 1.0f}, yellow},
	{ {1.0f, 1.0f, 0.0f}, blue},
	{ {0.0f, 2.0f, 0.0f}, blue},
	{ {0.0f, 1.0f, 1.0f}, blue},
	{ {2.0f, 0.0f, 0.0f}, red},
	{ {1.0f, 1.0f, 0.0f}, red},
	{ {1.0f, 0.0f, 1.0f}, red},
	{ {1.0f, 0.0f, 0.0f}, green},
	{ {0.0f, 1.0f, 0.0f}, green},
	{ {1.0f, 1.0f, 0.0f}, green},
	{ {2.0f, 0.0f, 0.0f}, yellow},
	{ {1.0f, 0.0f, 0.0f}, yellow},
	{ {1.0f, 1.0f, 0.0f}, yellow},
	{ {0.0f, 1.0f, 0.0f}, red},
	{ {0.0f, 2.0f, 0.0f}, red},
	{ {1.0f, 1.0f, 0.0f}, red},
	},
	{},
	&arrangePyramidVertex
};

botchedModel tutorial = {
	{
		{ {-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{ {0.5f, -0.5f, 0.0f}, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
		{ {0.5f, 0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }},
		{ {-0.5f, 0.5f, 0.0f}, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }},

		{ {-0.5f, -0.5f, -0.5f},{ 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
		{ {0.5f, -0.5f, -0.5f}, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
		{ {0.5f, 0.5f, -0.5f},  { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }},
		{ {-0.5f, 0.5f, -0.5f}, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }}
	},
	{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	},
	NULL
};



void generateVertices(std::vector<Vertex>& vert, std::vector<uint16_t>& ind) {
	botchedModel& m = tutorial;

	vert = m.v;
	ind = m.i;
	if (m.preSPT)
		m.preSPT(vert, ind);
}