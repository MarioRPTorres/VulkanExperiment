#ifndef IMGUI_VULKAN_H
#define IMGUI_VULKAN_H

#include "imgui.h"
#include "vulkan_engine.h"


struct VkEImgui_Backend {
	VulkanEngine* engine = VK_NULL_HANDLE;
	VkE_Image fontSImage;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSet fontDescriptorSet = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipelineCache pipelineCache = VK_NULL_HANDLE;
	VkShaderModule ShaderModuleVert = VK_NULL_HANDLE;
	VkShaderModule ShaderModuleFrag = VK_NULL_HANDLE;

	VkAllocationCallbacks* allocator = nullptr;
	VkPipelineCreateFlags pipelineCreateFlags = 0;
	uint32_t minImageCount;
	uint32_t imageCount;
	VkDeviceSize BufferMemoryAlignment;

	// Main Window renderer objects
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> frameBuffers;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> commandBuffers;

	// Viewport Renderer common objects
	BufferBundle vertexBuffer;
	BufferBundle indexBuffer;
	// Viewports
//	std::vector<VKEngine_Imgui_Viewport> viewports = {};

	VkEImgui_Backend()
	{
		memset((void*)this, 0, sizeof(*this));
		BufferMemoryAlignment = 256;
	}
};

struct VkEImgui_DeviceObjectsInfo {
	bool firstPass = false;
};

void check_vk_result(VkResult err); 

void VkEImgui_setupBackEnd(VkEImgui_Backend& bd, VulkanEngine* vk, uint32_t minImageCount, uint32_t imageCount);
void VkEImgui_init(VulkanEngine* vk, VkEImgui_Backend& imguiBackEnd);
void VkEImgui_createBackendObjects(VulkanEngine* vk, VkEImgui_Backend& imBd,VkEImgui_DeviceObjectsInfo info);
void createImguiDeviceObjects(VulkanEngine* vk, VkEImgui_Backend& imObj, VkEImgui_DeviceObjectsInfo info);
void VkEImgui_addDefaultFont(VkEImgui_Backend& imBd);
void VkEImgui_cleanupBackEndObjects(VkEImgui_Backend& imObj);
void VkEImgui_cleanupSwapChain(VkEImgui_Backend& imObj);
void recreateImguiSwapChainObjects(VkEImgui_Backend& imObj, VkEImgui_DeviceObjectsInfo info);
void VkEImgui_CreatePipeline(VkEImgui_Backend* bd);
void VkEImgui_Shutdown();

void VkE_Imgui_NewFrame();
void VkEImgui_RenderDrawData(ImDrawData* draw_data, VkCommandBuffer command_buffer, VkPipeline pipeline = VK_NULL_HANDLE);
#endif