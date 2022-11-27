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
	std::vector<Vertex> v;
	std::vector<uint32_t> i;
	void (*preSPT) (std::vector<Vertex>&, std::vector<uint32_t>&);
};

void generateVertices(std::vector<Vertex>& vert, std::vector<uint32_t>& ind);

#endif