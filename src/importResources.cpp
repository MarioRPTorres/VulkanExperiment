#include "importResources.h"

const float pi = 3.14159265358979323846264338327950288;

void arrangePyramidVertex(std::vector<Vertex>& vert, std::vector<uint32_t>& ind) {
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
		{ {-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, 0},
		{ {0.5f, -0.5f, 0.0f}, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f },0},
		{ {0.5f, 0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f },0},
		{ {-0.5f, 0.5f, 0.0f}, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f },0},

		{ {-0.5f, -0.5f, -0.5f},{ 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f },1},
		{ {0.5f, -0.5f, -0.5f}, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f },1},
		{ {0.5f, 0.5f, -0.5f},  { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f },1},
		{ {-0.5f, 0.5f, -0.5f}, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f },1}
	},
	{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	},
	NULL
};


void generateVertices(std::vector<Vertex>& vert, std::vector<uint32_t>& ind) {
	botchedModel& m = tutorial;

	vert = m.v;
	ind = m.i;
	if (m.preSPT)
		m.preSPT(vert, ind);
}
