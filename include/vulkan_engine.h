
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
// Number of parallel descriptor sets to switch between
const int MIRROR_DESCRIPTOR_SET_COUNT = 2;
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

typedef struct _SampledImage {
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
} SampledImage;


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
	VkFormat format;
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

struct VkE_createRenderPassInfo {
	VkFormat format;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	bool firstPass = true;
	bool finalPass = true;
	bool depthStencil = false;
};

struct BufferBundle {
	VkBuffer buffer;
	VkDeviceMemory memory;
};

struct vertexDescriptions {
	VkVertexInputBindingDescription binding;
	std::vector<VkVertexInputAttributeDescription> attributes;
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
protected:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	uint32_t graphicsFamily; // Graphics queue family
	uint32_t transferFamily; // Transfer queue family
	uint32_t mainPresentFamily; // Main Window Present Family queue family
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;
	VkSurfaceKHR mainSurface;

	VkE_SwapChain mainSwapChain;
	VkSwapchainKHR& swapChain = mainSwapChain.swapChain;
	std::vector<VkImage>& swapChainImages = mainSwapChain.images;
	std::vector<VkImageView>& swapChainImageViews = mainSwapChain.imageViews;
	VkFormat& swapChainImageFormat = mainSwapChain.format;
	VkExtent2D& swapChainExtent = mainSwapChain.extent;

	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	VkCommandPool transientcommandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	VkE_FrameSyncObjects syncObjects;
	std::vector<VkSemaphore>& imageAvailableSemaphore = syncObjects.imageAvailableSemaphore;
	std::vector<VkSemaphore>& renderFinishedSemaphore = syncObjects.renderFinishedSemaphore;
	std::vector<VkFence>& inFlightFences = syncObjects.inFlightFences;
	std::vector<VkFence>& imagesInFlight = syncObjects.imagesInFlight;

	size_t currentFrame = 0;
	VkDescriptorPool descriptorPool;
	std::array<std::vector<VkDescriptorSet>, MIRROR_DESCRIPTOR_SET_COUNT> descriptorSets;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	// Multisampling needs an offscreen buffer that is then rendered to the scrren
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	uint32_t indexCount = 0;

	uint32_t mipLevels;
	bool framebufferResized = false;


	
	// Initial setup. Not likely to change
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	int rateDeviceSuitability(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	void pickPhysicalDevice();
	void createLogicalDevice();


	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createDescriptorSetLayout();
	// For creating the graphics pipeline
	void createGraphicsPipeline(shaderCode vert, shaderCode frag, vertexDescriptions vertex);

	// Various Resources
	void createCommandPool();
	void createColorResources();
	void createDepthResources();

	void createIndexBuffer(std::vector<uint32_t> indices);

	// Descriptors
	void createDescriptorPool();
	void createDescriptorSets();
	

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	
	void createImage(uint32_t width, uint32_t height,
		uint32_t mipLevels, VkSampleCountFlagBits numSamples,
		VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image,
		VkDeviceMemory& imageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	VkSampleCountFlagBits getMaxUsableSampleCount();


public:
	void createSwapChain(VkSurfaceKHR surface, VkE_SwapChain& swapChainDetails);
	void createSwapChainImageViews(const std::vector<VkImage>& images, const VkFormat format, std::vector<VkImageView>& swapChainImageViews);
	VkRenderPass createRenderPass(VkE_createRenderPassInfo info);
	std::vector<VkFramebuffer> createFramebuffers(const VkRenderPass renderPass, const VkE_SwapChain& swapChain, VkImageView colorAttachment = VK_NULL_HANDLE, VkImageView depthAttachment = VK_NULL_HANDLE);
	VkShaderModule createShaderModule(const shaderCode& code);
	std::vector<VkCommandBuffer> createCommandBuffers(const VkCommandPool commandPool, uint32_t buffersCount);
	void freeDescriptorSet(VkDescriptorPool pool, VkDescriptorSet& set);
	void createImageSampler(VkSampler& sampler, uint32_t mipLevels);
	void createSampledImage(SampledImage& image, int cols, int rows, int elemSize, char* imageData, uint32_t mipLvls, VkSampleCountFlagBits numsamples);
	void cleanupSampledImage(SampledImage& image);
	void mapBufferMemory(VkDeviceMemory bufferMemory, void* data, VkDeviceSize datalen);
	void destroyBufferBundle(BufferBundle buffer);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void createSyncObjects(VkE_FrameSyncObjects& syncObjs, uint32_t imagesCount);
	BufferBundle VulkanEngine::createBufferWithData(void* data, VkDeviceSize bufferSize, VkFlags usage);

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

#endif