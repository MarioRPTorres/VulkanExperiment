#ifndef IMGUI_VULKAN_H
#define IMGUI_VULKAN_H
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "vulkan_engine.h"

#ifdef IMGUI_EXT
const bool enableImgui = true;
#else
const bool enableImgui = false;
#endif

struct VulkanImguiDeviceObjects {
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipeline graphicsPipeline = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> frameBuffers;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> commandBuffers;
};

void check_vk_result(VkResult err); 
void initImgui(VulkanEngine* vk, VulkanImguiDeviceObjects& imObj);
void createImguiDeviceObjects(VulkanEngine* vk, VulkanImguiDeviceObjects& imObj);
void cleanupImguiObjects(VkDevice device, VulkanImguiDeviceObjects& imObj);
void cleanupImguiSwapChainObjects(VkDevice device, VulkanImguiDeviceObjects& imObj);
void recreateImguiSwapChainObjects(VulkanEngine* vk, VulkanImguiDeviceObjects& imObj);
#endif