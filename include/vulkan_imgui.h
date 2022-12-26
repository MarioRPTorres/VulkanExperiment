#ifndef IMGUI_VULKAN_H
#define IMGUI_VULKAN_H

#include "imgui.h"
#include "vulkan_engine.h"


struct VulkanImgui_DeviceObjects {
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipeline graphicsPipeline = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> frameBuffers;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> commandBuffers;
};

struct VulkanImgui_DeviceObjectsInfo {
	bool firstPass = false;
};

void check_vk_result(VkResult err); 
void initImgui(VulkanEngine* vk, VulkanImgui_DeviceObjects& imObj);
void createImguiDeviceObjects(VulkanEngine* vk, VulkanImgui_DeviceObjects& imObj, VulkanImgui_DeviceObjectsInfo info);
void cleanupImguiObjects(VkDevice device, VulkanImgui_DeviceObjects& imObj);
void cleanupImguiSwapChainObjects(VkDevice device, VulkanImgui_DeviceObjects& imObj);
void recreateImguiSwapChainObjects(VulkanEngine* vk, VulkanImgui_DeviceObjects& imObj, VulkanImgui_DeviceObjectsInfo info);

void VKEngine_Imgui_NewFrame();
void VKEngine_Imgui_RenderDrawData(ImDrawData* draw_data, VkCommandBuffer command_buffer, VkPipeline pipeline = VK_NULL_HANDLE);
#endif