
#ifndef VULKAN_ENGINE_H
#define VULKAN_ENGINE_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <cstdint> // Necessary for UINT32_MAX
#include <algorithm> // For std::max/min


#include <vector>
#include <array>
#include <set>

#ifdef PHYSICAL_DEVICE_SCORE_SELECTION
#include <map>
#endif

#include "assert.h"
#define VKE_ASSERT(_EXPR)            assert(_EXPR)

// ****************** Constants ********************
// Number of frames to be processed concurrently(simultaneously)
const int MAX_FRAME_IN_FLIGHT = 2;
// How many textures we will have loaded at the same time
const int MAX_SAMPLED_IMAGES = 2;
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// ****************** Init variables ********************
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// ****************** Datatypes ********************
typedef std::vector<uint32_t> shaderCode;

typedef struct _VkE_Image {
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
} VkE_Image;


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

struct VulkanBackEndData {
	GLFWwindow* window;
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	uint32_t graphicsQueueFamily;
	VkQueue graphicsQueue;
	VkQueue transientQueue;
	VkCommandPool commandPool;
};

struct VkE_SwapChain {
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	uint32_t minImageCount;
	uint32_t imageCount;
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkExtent2D extent;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews; // missing
};

struct VkE_FrameSyncObjects {
	std::vector<VkSemaphore> imageAvailableSemaphore;
	std::vector<VkSemaphore> renderFinishedSemaphore;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
};

struct VkE_Buffer {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDeviceSize size = 0;
};

struct vertexDescriptions {
	VkVertexInputBindingDescription binding;
	const VkVertexInputAttributeDescription* data;
	uint32_t vertexDescriptionsCount;
};

// *************** Other Functions *************
void char2shaderCode(std::vector<char> inCharVector, shaderCode& outShaderCode);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

// ***************************** Vulkan Engine Class ****************************
// ******************************************************************************

// The VulkanEngine class follows the Singleton pattern. Defines the `GetInstance` method that serves as an
// alternative to constructor and lets clients access the same instance of this class over and over.
class VulkanEngine
{
	/**
	 * The Singleton's constructor should always be private to prevent direct
	 * construction calls with the `new` operator.
	 */
public: 
	VkDevice device;
	VkCommandPool transientcommandPool;
	uint32_t graphicsFamily; // Graphics queue family
	uint32_t transferFamily; // Transfer queue family
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;
	VkSampleCountFlagBits maxMSAASamples = VK_SAMPLE_COUNT_1_BIT;

protected:
	VkInstance instance;
	GLFWwindow* window;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	uint32_t mainPresentFamily; // Main Window Present Family queue family

	VkE_SwapChain mainSwapChain;

	VkRenderPass renderPass;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	VkE_FrameSyncObjects syncObjects;
	std::vector<VkSemaphore>& imageAvailableSemaphore = syncObjects.imageAvailableSemaphore;
	std::vector<VkSemaphore>& renderFinishedSemaphore = syncObjects.renderFinishedSemaphore;
	std::vector<VkFence>& inFlightFences = syncObjects.inFlightFences;
	std::vector<VkFence>& imagesInFlight = syncObjects.imagesInFlight;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	// Multisampling needs an offscreen buffer that is then rendered to the scrren
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	
	int rateDeviceSuitability(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device,VkSurfaceKHR surface);


	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	
	// Various Resources
	void createColorResources();
	void createDepthResources();

	

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkSampleCountFlagBits getMaxUsableSampleCount();


public:
	// Initial setup. Not likely to change
	void createInstance();
	void setupDebugMessenger();
	void pickPhysicalDevice(VkSurfaceKHR surface);
	void createLogicalDevice();

	void shutdownVulkanEngine();

	VkFormat findDepthFormat();
	void createSurface(GLFWwindow* window, VkSurfaceKHR& surface);
	inline void destroySurface(VkSurfaceKHR& surface) {
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}
	// SwapChain
	void createSwapChain(VkSurfaceKHR surface, VkE_SwapChain& swapChainDetails);
	void createSwapChainImageViews(const std::vector<VkImage>& images, const VkFormat format, std::vector<VkImageView>& swapChainImageViews);
	void createSwapChainImageViews(VkE_SwapChain& swapChainDetails);
	
	// For creating the graphics pipeline
	void createGraphicsPipeline(VkPipeline& graphicsPipeline, VkPipelineLayout& pipelineLayout, 
		VkRenderPass renderPass, 
		shaderCode vert, shaderCode frag, 
		vertexDescriptions vertex, 
		VkDescriptorSetLayout descriptorSetLayout, 
		VkExtent2D extent,
		VkSampleCountFlagBits rasterizationSamples);
	void createRenderPass(VkRenderPass& renderPass, VkFormat format, VkSampleCountFlagBits msaaSamples, bool firstPass, bool finalPass, bool depthStencil ,bool clearEnable);
	std::vector<VkFramebuffer> createFramebuffers(const VkRenderPass renderPass, const VkE_SwapChain& swapChain, VkImageView colorAttachment = VK_NULL_HANDLE, VkImageView depthAttachment = VK_NULL_HANDLE);
	VkShaderModule createShaderModule(const shaderCode& code);
	
	// Descriptors
	void createDescriptorPool(VkDescriptorPool& descriptorPool, uint32_t uniformBufferCount, uint32_t imageSamplersCount,uint32_t maxSets);
	void freeDescriptorSet(VkDescriptorPool pool, VkDescriptorSet& set);
	// Images
	void createImage(uint32_t width, uint32_t height,
		uint32_t mipLevels, VkSampleCountFlagBits numSamples,
		VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image,
		VkDeviceMemory& imageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void createImageSampler(VkSampler& sampler, uint32_t mipLevels);
	void createSampledImage(VkE_Image& image, int cols, int rows, int elemSize, char* imageData, uint32_t mipLvls, VkSampleCountFlagBits numsamples);
	void cleanupSampledImage(VkE_Image& image);
	

	// Buffers
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void destroyBufferBundle(VkE_Buffer buffer);
	void mapBufferMemory(VkDeviceMemory bufferMemory, void* data, VkDeviceSize datalen);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void createBufferWithData(void* data, VkDeviceSize bufferSize, VkFlags usage, VkE_Buffer& buffer);
	inline void createVertexBuffer(void* data, VkDeviceSize bufferSize, VkE_Buffer& vertexBuffer) {
		createBufferWithData(data, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer);
	};
	inline void createIndexBuffer(void* data, VkDeviceSize bufferSize, VkE_Buffer& indexBuffer) {
		createBufferWithData(data, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer);
	};

	// Command Buffers
	void createCommandPool(VkCommandPool& pool, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags);
	std::vector<VkCommandBuffer> createCommandBuffers(const VkCommandPool commandPool, uint32_t buffersCount);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	// Syncronization objects
	void createSyncObjects(VkE_FrameSyncObjects& syncObjs, uint32_t imagesCount);
	void cleanupSyncObjects(VkE_FrameSyncObjects& syncObjs);

	VulkanBackEndData getBackEndData() {
		VulkanBackEndData bd = {
			window,
			instance,
			physicalDevice,
			device,
			graphicsFamily,
			graphicsQueue,
			transferQueue,
			commandPool
		};

		return bd;
	}

	inline VkE_SwapChain* getSwapChainDetails() { return &mainSwapChain; }

};

// ***************************** Vulkan Window Class ****************************
// ******************************************************************************
class VulkanWindow {
protected:
	VulkanEngine* vk;
	GLFWwindow* window = nullptr;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkE_SwapChain sc;
	std::vector<VkFramebuffer> frameBuffers;
	std::vector<VkCommandBuffer> commandBuffers;
	VkE_FrameSyncObjects syncObjects;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;

	bool ClearEnable = false;
	VkClearValue clearValue;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	uint32_t mipLevels = 1;
	bool framebufferResized = false;
	bool swapChainOutdated = false;
	uint32_t imageIndex = 0;
	uint32_t inFlightFrameIndex = 0;
	VkE_Image msaaColorImage;
	VkE_Image depthImage;

	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	float width = 0;
	float height = 0;
protected:
	// Various Resources
	void createColorResources();
	void createDepthResources();

public:
	VulkanWindow(VulkanEngine* vk) : vk(vk){};
};
#endif