#ifndef IMPORT_RESOURCES_H
#define IMPORT_RESOURCES_H

#include "vulkan_engine.h"
#include "vulkan_vertices.h"

const glm::vec3 red = { 1.0f,0.0f,0.0f };
const glm::vec3 green = { 0.0f,1.0f,0.0f };
const glm::vec3 blue = { 0.0f,0.0f,1.0f };
const glm::vec3 yellow = { 1.0f,1.0f,0.0f };
const glm::vec3 white = { 1.0f,1.0f,1.0f };


struct botchedModel {
	std::vector<PCTVertex> v;
	std::vector<uint32_t> i;
	void (*preSPT) (std::vector<PCTVertex>&, std::vector<uint32_t>&);
};

void generateVertices(std::vector<PCTVertex>& vert, std::vector<uint32_t>& ind);

#endif