#ifndef IMGUI_VULKAN_H
#define IMGUI_VULKAN_H

#include "vulkan_engine.h"

struct VkEImgui_vertexBuffers {
	VkE_Buffer vertex;
	VkE_Buffer index;
};

struct VkEImgui_Viewport {
	GLFWwindow* window = nullptr; // Not used in main window
	VkSurfaceKHR surface = VK_NULL_HANDLE; // Not used in main window
	VkE_SwapChain sc; // Not used in main window
	std::vector<VkFramebuffer> frameBuffers;
	std::vector<VkCommandBuffer> commandBuffers;
	VkE_FrameSyncObjects syncObjects; // Not used in main window
	VkQueue presentQueue; // Not used in main window
	VkCommandPool commandPool; 
	bool ClearEnable;
	bool WindowOwned = false;
	std::vector<VkEImgui_vertexBuffers> vertexBuffers;
	
	VkClearValue clearValue;
	bool swapChainOutdated = false;
	uint32_t imageIndex = 0;
	uint32_t inFlightFrameIndex = 0; // Not used in main window

	VkRenderPass renderPass;
	VkPipeline pipeline;
	float width;
	float height;
};

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
	uint32_t maxFramesInFlight = 0;
	VkDeviceSize BufferMemoryAlignment;

	// Main Window renderer objects
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> frameBuffers;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> commandBuffers;

	// Viewport Renderer common objects
	//VkRenderPass vpRenderPass = VK_NULL_HANDLE;
	//VkPipeline vpPipeline = VK_NULL_HANDLE;
	// Viewports
	VkEImgui_Viewport mainViewport;
	std::vector<VkEImgui_Viewport*> viewports = {};

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

void VkEImgui_setupBackEnd(VkEImgui_Backend& bd, VulkanEngine* vk, uint32_t minImageCount, uint32_t imageCount, uint32_t maxFramesInFlight);
void VkEImgui_init(VulkanEngine* vk, VkEImgui_Backend& imguiBackEnd);
void VkEImgui_createBackEndObjects(VulkanEngine* vk, VkEImgui_Backend& imBd,VkEImgui_DeviceObjectsInfo info);
void VkEImgui_addDefaultFont(VkEImgui_Backend& imBd);
void VkEImgui_cleanupBackEndObjects(VkEImgui_Backend& imObj);
void VkEImgui_cleanupSwapChain(VkEImgui_Backend& imObj);
void recreateImguiSwapChainObjects(VkEImgui_Backend& imObj, VkEImgui_DeviceObjectsInfo info);
void VkEImgui_CreatePipeline(VkEImgui_Backend* bd);
void VkEImgui_Shutdown();

void VkE_Imgui_NewFrame();
void VkEImgui_RenderDrawData(void* imgui_draw_data, VkCommandBuffer command_buffer, VkPipeline pipeline = VK_NULL_HANDLE);
#endif