
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

#include <opencv2/opencv.hpp>

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
typedef std::vector<char> shaderCode;

typedef struct _SampledImage {
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
} SampledImage;

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

struct VulkanBackEndData {
	GLFWwindow* window;
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	uint32_t graphicsQueueFamily;
	VkQueue graphicsQueue;
	VkQueue transientQueue;
};

struct SwapChainDetails {
	uint32_t minImageCount;
	uint32_t imageCount;
	VkFormat format;
	VkExtent2D extent;
	std::vector<VkImageView> imageViews;
};

// *************** Other Functions *************
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

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
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	VkCommandPool transientcommandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphore;
	std::vector<VkSemaphore> renderFinishedSemaphore;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;
	VkDescriptorPool descriptorPool;
	std::array<std::vector<VkDescriptorSet>, MIRROR_DESCRIPTOR_SET_COUNT> descriptorSets;
	int descriptorGroup = 0;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	// Multisampling needs an offscreen buffer that is then rendered to the scrren
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	uint32_t indexCount = 0;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	uint32_t mipLevels;
	bool framebufferResized = false;


	
	// Initial setup. Not likely to change
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	int rateDeviceSuitability(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void pickPhysicalDevice();
	void createLogicalDevice();


	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void createSwapChainImageViews();
	void createRenderPass();
	void createDescriptorSetLayout();
	// For creating the graphics pipeline
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createGraphicsPipeline(shaderCode vert, shaderCode frag);

	// Various Resources
	void createCommandPool();
	void createCommandBuffers();
	void createColorResources();
	void createDepthResources();
	void createFramebuffers();
	void createSyncObjects();

	void createVertexBuffer(std::vector<Vertex> vertices);
	void createIndexBuffer(std::vector<uint32_t> indices);

	// Descriptors
	void createDescriptorPool();
	void createDescriptorSets();
	void createUniformBuffers();
	void createSampledImage(SampledImage& image, std::string imageFile);
	void updateDescriptorSet(std::array<SampledImage, MAX_SAMPLED_IMAGES> images, int groupIndex);
	void writeCommandBuffers();

	

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	
	void createImage(uint32_t width, uint32_t height,
		uint32_t mipLevels, VkSampleCountFlagBits numSamples,
		VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image,
		VkDeviceMemory& imageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void createImageSampler(VkSampler& sampler, uint32_t mipLevels);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	VkSampleCountFlagBits getMaxUsableSampleCount();

	void cleanupSampledImage(SampledImage& image);

public:
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	
	VulkanBackEndData getBackEndData() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
		VulkanBackEndData bd = {
			window,
			instance,
			physicalDevice,
			device,
			queueFamilyIndices.graphicsFamily.value,
			graphicsQueue,
			transferQueue
		};

		return bd;
	}

	SwapChainDetails getSwapChainDetails() {
		SwapChainSupportDetails sc = querySwapChainSupport(physicalDevice);
		SwapChainDetails details = {
			sc.capabilities.minImageCount,
			static_cast<uint32_t> (swapChainImages.size()),
			swapChainImageFormat,
			sc.capabilities.currentExtent,
			swapChainImageViews
		};

		return details;
	}

};

#endif