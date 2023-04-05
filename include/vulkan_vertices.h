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

// Size of attributtes are specified via the format
//• float: VK_FORMAT_R32_SFLOAT
//• vec2 : VK_FORMAT_R32G32_SFLOAT
//• vec3 : VK_FORMAT_R32G32B32_SFLOAT
//• vec4 : VK_FORMAT_R32G32B32A32_SFLOAT

namespace constants
{
	// actual global variables
	extern const std::array<VkVertexInputAttributeDescription, 4> PCTVertexAttributes;
}

struct PCTVertex {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 color;
	alignas(8) glm::vec2 texCoord;
	alignas(4) int texId;

	static VkVertexInputBindingDescription getBindingDescription() {
		// Binding Description tells the rate at which vertex data is coming in
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;								// Binding index of this binding in the binding array
		bindingDescription.stride = sizeof(PCTVertex);					// Stride between each vertex data 
		// ï¿½ VK_VERTEX_INPUT_RATE_VERTEX : Move to the next data entry after each vertex
		// ï¿½ VK_VERTEX_INPUT_RATE_INSTANCE : Move to the next data entry after each instance
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	bool operator==(const PCTVertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}

	static vertexDescriptions getDescriptions() {
		return vertexDescriptions({
			getBindingDescription(),
			constants::PCTVertexAttributes.data(),
			constants::PCTVertexAttributes.size()
		});
	}
};

#endif