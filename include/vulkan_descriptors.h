#ifndef VULKAN_DESCRIPTORS_H
#define VULKAN_DESCRIPTORS_H

#include "vulkan_engine.h"

#ifdef INCLUDE_GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp> // For Vertex hashing

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};
#endif

VkDescriptorSet createSingleImageDecriptorSet(VulkanEngine* vk, VkDescriptorPool pool, VkDescriptorSetLayout layout, VkE_Image image);
#endif