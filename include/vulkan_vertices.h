#ifndef VULKAN_VERTICES_H
#define VULKAN_VERTICES_H

#include "vulkan_engine.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp> // For Vertex hashing

struct Vertex {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 color;
	alignas(8) glm::vec2 texCoord;
	alignas(4) int texId;


	static VkVertexInputBindingDescription getBindingDescription() {
		// Binding Description tells the rate at which vertex data is coming in
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;								// Binding index of this binding in the binding array
		bindingDescription.stride = sizeof(Vertex);					// Stride between each vertex data 
		// • VK_VERTEX_INPUT_RATE_VERTEX : Move to the next data entry after each vertex
		// • VK_VERTEX_INPUT_RATE_INSTANCE : Move to the next data entry after each instance
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	static std::array<VkVertexInputAttributeDescription, 4>getAttributeDescriptions() {
		// This struct tells how to extract a vertex attribute from a vertex. Since we have two attributes, position and color, we have an array of two structs.
		std::array<VkVertexInputAttributeDescription, 4>attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		//• float: VK_FORMAT_R32_SFLOAT
		//• vec2 : VK_FORMAT_R32G32_SFLOAT
		//• vec3 : VK_FORMAT_R32G32B32_SFLOAT
		//• vec4 : VK_FORMAT_R32G32B32A32_SFLOAT
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);


		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32_SINT;
		attributeDescriptions[3].offset = offsetof(Vertex, texId);
		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}

	static vertexDescriptions getDescriptions() {
		auto attributes = getAttributeDescriptions();
		return vertexDescriptions({ getBindingDescription(), std::vector<VkVertexInputAttributeDescription>(attributes.begin(), attributes.end()) });
	}
};

BufferBundle createVertexBuffer(VulkanEngine* vk, std::vector<Vertex> vertices);


#endif