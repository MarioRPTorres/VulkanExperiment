
#ifndef MAIN_H
#define MAIN_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <cstdint> // Necessary for UINT32_MAX
#include <algorithm> // For std::max/min
#include <opencv2/opencv.hpp>

// To include the functions bodies and avoid linker errors
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map> // To load the models 
 
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp> // For Vertex hashing

#include <chrono>

#include <vector>
#include <array>
#include <set>

#define PHYSICAL_DEVICE_SCORE_SELECTION

#ifdef PHYSICAL_DEVICE_SCORE_SELECTION
#include <map>
#endif

struct Vertex {
	alignas(16) glm::vec3 pos;
	alignas(16) glm::vec3 color;
	alignas(8) glm::vec2 texCoord;

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
	static std::array<VkVertexInputAttributeDescription, 3>getAttributeDescriptions() {
		// This struct tells how to extract a vertex attribute from a vertex. Since we have two attributes, position and color, we have an array of two structs.
		std::array<VkVertexInputAttributeDescription, 3>attributeDescriptions = {};
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
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct optional {
	uint32_t value;
	bool has_value;
	optional() : value(0), has_value(false) {};
	void set_value(uint32_t v) {
		value = v;
		has_value = true;
	};
	void reset_value() {
		value = 0;
		has_value = false;
	}
};

struct QueueFamilyIndices {
	optional graphicsFamily;
	optional presentFamily;
	optional transferFamily;

	bool isComplete() {
		return graphicsFamily.has_value && presentFamily.has_value && transferFamily.has_value;
	}

	bool sharedTransfer() {
		return (graphicsFamily.has_value && transferFamily.has_value && (graphicsFamily.value == transferFamily.value));
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


void loadModel();

#endif