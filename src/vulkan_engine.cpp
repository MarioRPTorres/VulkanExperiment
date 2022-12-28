#include "vulkan_engine.h"

//********************** External Functions ****************************
//**********************************************************************

//**************************** Helper Functions ****************************
void char2shaderCode(std::vector<char> inCharVector, shaderCode& outShaderCode) {
	outShaderCode.clear();
	size_t intCount = (int)(inCharVector.size() * sizeof(char) / sizeof(uint32_t));
	outShaderCode = shaderCode((uint32_t*)inCharVector.data(), (uint32_t*)inCharVector.data() + intCount);
	return;
}

///// Debug CallBack
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,		// Message Severity Level
	VkDebugUtilsMessageTypeFlagsEXT messageType,				// Message Type 
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,	// Struct pointer to Important information
																// regarding the message such as
																// pMessage -> Debug Message as null-terminated string
																// pObjects -> Array of Vulkan Objects related to message
																// objectCount -> number of objects in array
	void* pUserData) 											// Pointer to UserDefined data setup during Callback
{
	// Debug Messenger Function Callback
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		// Message is important enough to show
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	}

	// Returns boolean telling wether to abort the Vulkan call that triggered this function or not.
	// Should be always VK_FALSE
	return VK_FALSE;

};

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,VkDebugUtilsMessengerEXT* pDebugMessenger) {

	// Loads the function to create the Debug Messenger object.
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr) {
		// If the function is valid, a call to it is made to create the Debug Messenger object and
		// pass the said object's handle to the pointer
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,VkDebugUtilsMessengerEXT debugMessenger,const VkAllocationCallbacks* pAllocator) {
	// Similar to the CreateDebugUtilsMessengerEXT function, it loads the deletion function of 
	// the debug Messenger Object and makes a call to it using said object.

	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}

}

// Automatic Population of a DebugMessengerCreateInfo struct
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	// Defines all the types of severities to trigger the callback
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	// Same as Severity field 
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; // Optional
}

static bool hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

bool checkValidationLayerSupport() {
	/* Searches the available validation layers for the need validation layers
	described in validationLayers.
	Returns true if all are available, false otherwise.*/
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

std::vector<const char*> getRequiredExtensions() {
	// GLFW extension information given by GLFW so Vulkan knows how to interact with the windows system(GLFW)
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	// Add the Debug Messenger Extension if validation layers are enabled
	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return extensions;
}

bool checkExtensionSupport(std::vector<const char*> Extensions) {
	// To check if other extensions are functional first get the number of extension available
	uint32_t availableExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
	// Allocate for the number of extensions
	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	// Query the available extensions
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

	std::cout << "Extension Needed:" << std::endl;

	//for (uint32_t i = 0; i < ExtensionCount; i++) {
	for (const char* extension : Extensions) {
		bool extensionFound = false;
		//const char* extension = Extensions[i];

		std::cout << "\t" << extension;

		for (const auto& availableExtension : availableExtensions) {
			if (strcmp(extension, availableExtension.extensionName) == 0) {
				extensionFound = true;
				std::cout << " Checked" << std::endl;
				break;
			}
		}

		if (!extensionFound) {
			std::cout << " Missing" << std::endl;
			return false;
		}

	}

	return true;

}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
	// Check device extensions for the list needed support
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	return requiredExtensions.empty();
}

//**************************** VulkanEngine ****************************
//**********************************************************************

void VulkanEngine::cleanupSampledImage(SampledImage& image) {
	if (image.sampler != VK_NULL_HANDLE) vkDestroySampler(device, image.sampler, nullptr);
	if (image.view != VK_NULL_HANDLE) vkDestroyImageView(device, image.view, nullptr);
	if (image.image != VK_NULL_HANDLE) vkDestroyImage(device, image.image, nullptr);
	if (image.memory != VK_NULL_HANDLE) vkFreeMemory(device, image.memory, nullptr);
}

void VulkanEngine::createInstance() {
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// Optional Info Struct for possible optimization information for the driver
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;



	// Fetch Required GLFW Extensions and extra Extensions requested
	auto extensions = getRequiredExtensions();

	// Check Required Extensions availability with Vulkan
	if (enableValidationLayers && !checkExtensionSupport(extensions)) {
		throw std::runtime_error("GLFW Extension not available!");
	}

	// Mandatory Information Struct that tells the Vulkan Driver which global 
	// extensions and validation layers we want to use
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// The debugMessengerObject creation info is place here to ensure it's not destroyed
	// before the vkCreateInstance call
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	// Enabled, it adds the validation layers to the create Instance info
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		// Adds a debugMessengerObject for the Creation of the Vulkan instance
		// This a seperate debugMessengerObject used for the application since that one is dependend 
		// on the Vulkan instance itself and can't debug its creation or destruction
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	}
	else {
		// Global validation layers are disabled;
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	// Create Vulkan Instance 
	// Here there is a choice for the Allocator function
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void VulkanEngine::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	// Info Structure to create the Debug Messenger
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	populateDebugMessengerCreateInfo(createInfo);

	// To create the Debug Messenger Object we have to load the Function that creates it since 
	// its an extension function and therefor not loaded
	// Here there is a choice for the Allocator function
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		throw std::runtime_error("failed to set up debug messenger!");

	return;
}

void VulkanEngine::createSurface() {
	// Here a Window Surface object is created for Vulkan to interact the window system. The necessary extensions
	// were already added by the glfwGetRequiredInstanceExtensions call. While the window surface object is agnostic
	// its creation isn't. Here the glfw already has a function that handles plataform specific problems for Vulkan
	// so the code remains platform agnostic. Even so, for curiosity, here is the Windows platform specific code 
	/*  // Windows window surface creation
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = glfwGetWin32Window(window);
		createInfo.hinstance = GetModuleHandle(nullptr);
		if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr,&surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	*/

	// Here there is a choice for the Allocator function
	if (glfwCreateWindowSurface(instance, window, nullptr, &mainSurface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create main window surface!");
	}
}

#ifdef PHYSICAL_DEVICE_SCORE_SELECTION
int VulkanEngine::rateDeviceSuitability(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int score = 0;

	// Discrete GPUs have a significant performance advantage
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}

	// Maximum possible size of textures affects graphics quality 
	score += deviceProperties.limits.maxImageDimension2D;

	// Further score checks can be made to improve performance.
	// Example: Implement code to choose a device with a specific combination of queues families based 
	// on their supported commands.

	// Application can't function without geometry shaders
	if (!isDeviceSuitable(device)) {
		return 0;
	}

	return score;
}
#endif

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}


	return details;
}

bool VulkanEngine::isDeviceSuitable(VkPhysicalDevice device) {
	// Checks for Properties and Features supported by the device as well as the QueueFamilies attributes
	//VkPhysicalDeviceProperties deviceProperties;
	//vkGetPhysicalDeviceProperties(device, &deviceProperties);

	//VkPhysicalDeviceFeatures deviceFeatures;
	//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// Example: Check if there is a Dedicated GPU that supports geometry shaders
	//bool isSuitable = (bool)(deviceProperties.deviceType == VK_PHYSICIAL_DEVICE_TYPE_DISCRETE_GPU &&
	//						deviceFeatures.geometryShader);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	// For windows renderings applications a surface is needed and swapchain support from the physical device is needed.
	// Not only the extension need to be checked and enabled by the device but the supported swapchain needs to be queried
	// for certains details to check if it is valid.
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device,mainSurface);
		// For this tutorial is sufficient to have at least one supported Image format and one supported presentation mode
		// for the given window surface.
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	// Pick device mandatory device features
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return findQueueFamilies(device,mainSurface).isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;
	optional transferFamilyIndice;

	// Logic to find queue family indices to populate struct with
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// Finds if the required queues families are supported by this device
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		// Check for graphics command support
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily.set_value(i);
		}

		// Checks for presentation support for the window surface
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport)
			indices.presentFamily.set_value(i);

		// Query for a queue with tranfer operation but not for graphics operations
		VkQueueFlags transferBit = queueFamily.queueFlags & (VK_QUEUE_TRANSFER_BIT | VK_QUEUE_GRAPHICS_BIT);
		if (transferBit == VK_QUEUE_TRANSFER_BIT)
			indices.transferFamily.set_value(i);
		else if (transferBit & VK_QUEUE_TRANSFER_BIT)
			transferFamilyIndice.set_value(i);

		if (indices.isComplete())
			break;

		i++;
	}

	if (!indices.transferFamily.has_value && transferFamilyIndice.has_value)
		indices.transferFamily.set_value(transferFamilyIndice.value);

	// The indeces here represent each queue family. Most likely a lot of the requirements will be met by the same
	// queue Family but throughout the code when a feature is required, there will be an individual index to call 
	// a queue family even if it's repeated. However there is a benefit of improved perfomance in choosing a device
	// with a minimal or specific combination of queues families. Further code can be added to the score function
	// to help improve perfomance. Example: Pick a device with drawing and presentation support in the same queue

	return indices;
}

void VulkanEngine::pickPhysicalDevice() {
	// List the available graphics card
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan Support!");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
#ifdef PHYSICAL_DEVICE_SCORE_SELECTION
	// Pick a Device based on a score rating
	// Use an ordered map to automatically sort candidates by
	// increasing score
	std::multimap<int, VkPhysicalDevice> candidates;

	for (const auto& device : devices) {
		int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}
	// Check if the best candidate is suitable at all
	if (candidates.rbegin()->first > 0) {
		physicalDevice = candidates.rbegin()->second;
	}
#else
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			break;
		}
	}
#endif

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
	// Add the multisample anti-aliasing count
	msaaSamples = getMaxUsableSampleCount();
}

void VulkanEngine::createLogicalDevice() {
	// Logical devices don't interact directly with instances

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice,mainSurface);
	if (indices.graphicsFamily.has_value) graphicsFamily = indices.graphicsFamily.value; else throw std::runtime_error("failed to choose graphics queue family!");
	if (indices.transferFamily.has_value) transferFamily = indices.transferFamily.value; else throw std::runtime_error("failed to choose transfer queue family!");
	if (indices.presentFamily.has_value) mainPresentFamily = indices.presentFamily.value; else throw std::runtime_error("failed to choose main window present queue family!");
	
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	// Set of unique Queue Family index values. There is no repetition of indeces in a set.
	std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily , mainPresentFamily, transferFamily };

	float queuePriority = 1.0f;
	// Loop over all necessary queueFamilies to create a vector of VkDeviceQueueCreationInfo
	// This way using a set, will create a vector with only the necessary number of CreationInfo structs without
	// repeating queue families.
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		// Creation Info of a single Queue Family for the Logical Device
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		// To schedule the command buffer execution of queues command, each queue is assigned a float from 0 to 1
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);

	}



	// Info on the Device Features to use
	VkPhysicalDeviceFeatures deviceFeatures = {};
	// Enable anisotrophy filtering for samplers. Almost all devices support this feature
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	// Logical Device Create Info struct
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	// Point to an array of Queue Families Create Info
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t> (queueCreateInfos.size());

	// Physical Device feature to request support
	createInfo.pEnabledFeatures = &deviceFeatures;

	// No extensions are needed for now
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// Previous implementations required a separate validation Layer for each device separate from
	// the Vulkan Instance Validation Layer. Up to date Implementations no longer make that distinction.
	// To keep previous compatibility the properties are still setup here
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	// Finally create the device and check for errors
	// Here there is a choice for the Allocator function
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, mainPresentFamily, 0, &presentQueue);
	// Transfer Challenge:
	// Use a different queue family and queue specifically for transfer operations.
	vkGetDeviceQueue(device, transferFamily, 0, &transferQueue);
}

VkSurfaceFormatKHR VulkanEngine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	// Choose the formats of the image such as how pixels are organised, the amount of bits per channel
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanEngine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	// Choose the method of queuing images in the swapchain to the window surface. It differentiates on how to handle
	// full queue and choosing images from the queue.
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanEngine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};


		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void VulkanEngine::createSwapChain(VkSurfaceKHR surface, VkE_SwapChain& swapChainDetails) {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice,surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	// Number of Layers per Image. Always one unless for stereoscopic 3D application
	createInfo.imageArrayLayers = 1;

	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice,surface);
	uint32_t queueFamilyIndices[] = { graphicsFamily,indices.presentFamily.value };

	// Sharing Mode of images across queue families
	//• VK_SHARING_MODE_EXCLUSIVE : An image is owned by one queue family
	//	at a time and ownership must be explicitly transfered before using it in
	//	another queue family.This option offers the best performance.
	//• VK_SHARING_MODE_CONCURRENT : Images can be used across multiple queue
	//	families without explicit ownership transfers.
	// To avoid involving ownership concepts we choose Concurrent mode.
	if ( graphicsFamily != indices.presentFamily.value) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		// An image is owned by one queue family
		// at a time and ownership must be explicitly transfered before using it in
		// another queue family.This option offers the best performance.
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}
	//  We can specify that a certain transform should be applied to images in the
	//	swap chain if it is supported(supportedTransforms in capabilities), like a
	//	90 degree clockwise rotation or horizontal flip.To specify that you do not want
	//	any transformation, simply specify the current transformation.
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	// Use Alpha channels to blend with other windows
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Here there is a choice for the Allocator function
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChainDetails.swapChain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swap chain!");
	}
	else {
		printf("Successfuly created swap chain\n");
	}

	// Get the swap chain image handles. We only specified the minimum Image necessary for the swap chain so we 
	// still need to query it for the number of images;
	vkGetSwapchainImagesKHR(device, swapChainDetails.swapChain, &swapChainDetails.imageCount, nullptr);
	swapChainDetails.images.resize(swapChainDetails.imageCount);
	vkGetSwapchainImagesKHR(device, swapChainDetails.swapChain, &swapChainDetails.imageCount, swapChainDetails.images.data());

	swapChainDetails.minImageCount = swapChainSupport.capabilities.minImageCount;
	swapChainDetails.format = surfaceFormat.format;
	swapChainDetails.extent = extent;

}

void VulkanEngine::createSwapChainImageViews(const std::vector<VkImage>& images,const VkFormat format,std::vector<VkImageView>& swapChainImageViews) {
	
	swapChainImageViews.resize(images.size());

	for (size_t i = 0; i < images.size(); i++) {
		swapChainImageViews[i] = createImageView(images[i], format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

VkRenderPass VulkanEngine::createRenderPass(VkE_createRenderPassInfo info) {
	bool multipleSamples = info.msaaSamples > VK_SAMPLE_COUNT_1_BIT;

	// Color Attachment creation
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = info.format;
	colorAttachment.samples = info.msaaSamples;
	//• VK_ATTACHMENT_LOAD_OP_DONT_CARE : Existing contents are undefined;
	//• VK_ATTACHMENT_LOAD_OP_LOAD : Preserve the existing contents of the attachment
	//• VK_ATTACHMENT_LOAD_OP_CLEAR : Clear the values to a constant at the	start
	colorAttachment.loadOp = (info.firstPass ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD); // Whether we want to keep what is already in the framebuffers
	//• VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering operation
	//• VK_ATTACHMENT_STORE_OP_STORE : Rendered contents will be stored in memory and can be read later
	//We’re interested in seeing the rendered triangle on the screen, so we’re going with the store operation here.
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//Textures and framebuffers in Vulkan are represented by VkImage objects with a certain pixel format,	
	//however the layout of the pixels in memory can change based on what you’re trying to do with an image.
	//• VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : Images used as color attachment
	//• VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : Images to be presented in the swap chain
	//• VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : Images to be used as destination for a memory copy operation
	colorAttachment.initialLayout = (info.firstPass ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	// Without msaa there is no need for more color attachment
	colorAttachment.finalLayout = (multipleSamples || !info.finalPass ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR); 

	// Reference to point to the attachment in the attachment array
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	// Attachments to submit
	std::vector<VkAttachmentDescription> attachments = { colorAttachment };

	// Depth Stencil
	VkAttachmentReference* pDepthAttachmentRef = nullptr;
	if (info.depthStencil) {
		// Depth Attachment creation
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = info.msaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
		attachments.push_back(depthAttachment);

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		pDepthAttachmentRef = &depthAttachmentRef;
	}

	// Color resolve for multiple samples
	VkAttachmentReference* pColorAttachmentResolveRef = nullptr;
	if ( multipleSamples ) {
		// Color Resolve Attachment creation
		VkAttachmentDescription colorAttachmentResolve = {};
		colorAttachmentResolve.format = info.format;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = info.finalPass ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attachments.push_back(colorAttachmentResolve);

		// Reference to point to the attachment in the attachment array
		VkAttachmentReference colorAttachmentResolveRef = {};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		pColorAttachmentResolveRef = &colorAttachmentResolveRef;
	}



	// Create first subpass with one attachment
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = pDepthAttachmentRef;
	subpass.pResolveAttachments = pColorAttachmentResolveRef;

	//The following other types of attachments can be referenced by a subpass :
	//• pInputAttachments : Attachments that are read from a shader
	//• pDepthStencilAttachment : Attachment for depth and stencil data
	//• pResolveAttachments : Attachments used for multisampling color attachments
	//• pPreserveAttachments : Attachments that are not used by this subpass,	but for which the data must be preserved


	// Create the Render pass with arguments of the subpasses used and the attachments to refer to.
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	// Subpass dependecies
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	//The first two fields specify the indices of the dependency and the dependent subpass.
	//The special value VK_SUBPASS_EXTERNAL refers to the implicit subpass before or after the render 
	//pass depending on whether it is specified in srcSubpass or dstSubpass.The index 0 refers to our
	//subpass, which is the first and only one.The dstSubpass must always be higher than srcSubpass to 
	//prevent cycles in the dependency graph
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkRenderPass renderPass;
	// Here there is a choice for the Allocator function
	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

	return renderPass;
}

void VulkanEngine::createDescriptorSetLayout() {
	// Create a descriptor which is used by the shader to access resources like images and buffers
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	// It is possible for the shader variable
	// to represent an array of uniform buffer objects, and descriptorCount specifies
	// the number of values in the array.This could be used to specify a transformation
	// for each of the bones in a skeleton for skeletal animation, for example.
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	// For image sampling related descriptors
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = static_cast<uint32_t>(MAX_SAMPLED_IMAGES);
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

VkShaderModule VulkanEngine::createShaderModule(const shaderCode& code) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size() * sizeof(code.at(0));
	// The byte code must be in uint32_t format with the same alignment requirements. Luckily the std::vector
	// default allocator already meet the worst case alignment requirements.
	createInfo.pCode = code.data();

	VkShaderModule shaderModule;
	// Here there is a choice for the Allocator function
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
	else {
		printf("Successfuly created Shader Module!\n");
	}

	return shaderModule;
}

void VulkanEngine::createGraphicsPipeline(shaderCode vert, shaderCode frag, vertexDescriptions vertex) {

	// Shader modules are only a thin wrapper around the shader bytecode. 
	// As soon as the graphics pipeline is created the shader modules are no longer needed
	// hence these can be made as local variables instead of class members. They must be 
	// destroyed by as call of vkDestroyShaderModule
	VkShaderModule vertShaderModule = createShaderModule(vert);
	VkShaderModule fragShaderModule = createShaderModule(frag);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	// Tells which pipeline stage the shader is going to be used.
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	/*	There is one more(optional) member, pSpecializationInfo, which we won’t
		be using here, but is worth discussing.It allows you to specify values for shader
		constants.You can use a single shader module where its behavior can be configured
		at pipeline creation by specifying different values for the constants used in
		it.This is more efficient than configuring the shader using variables at render
		time, because the compiler can do optimizations like eliminating if statements
		that depend on these values.*/
	vertShaderStageInfo.pSpecializationInfo = nullptr;

	// Now the same for the fragment shader
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	// Tells which pipeline stage the shader is going to be used.
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo,fragShaderStageInfo };


	// Prebuilt Fixed Functions Stage
	// Vertex Input
	// This stage tells how to load the vertex data and describes the format of the atributes as well spacing 
	// and per-vertex and per-instance information.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertex.binding;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex.attributes.size());
	vertexInputInfo.pVertexAttributeDescriptions = vertex.attributes.data();


	// Input Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	//		VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
	//		VK_PRIMITIVE_TOPOLOGY_LINE_LIST : line from every 2 vertices without reuse
	//		VK_PRIMITIVE_TOPOLOGY_LINE_STRIP : the end vertex of every line is used as start vertex for the next line
	//		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : triangle from every 3 vertices without reuse
	//		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP : the second and third vertex of every triangle are used as first two
	//	vertices of the next triangle
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewports
	// A viewport basically describes the region of the framebuffer that the output will
	// be rendered to.This will almost always be(0, 0) to (width, height)
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0;

	// Scissor 
	// Describes which pixels will actually be stored. In essence cuts the display of the framebuffer
	// including the viewport region.
	// Here we will draw on the entire framebuffer so the scissor should cover all of it.
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = swapChainExtent;

	// Now to combine both the viewport and scissor to a viewport state. 
	// Multiple viewports and scissors rectangle can be combined on some graphics card hence these will be 
	// passed as array. It requires enabling a GPU Feature in logical device creation.
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// If this is true then fragments beyond the near and far plane are clamped instead of discarded. 
	// Useful for things like shadow maps. It requires enabling a GPU Feature in logical device creation.
	rasterizer.depthClampEnable = VK_FALSE;
	// This if true disables any output to the framebuffer. The Geometry never passes through the rasterizer stage.
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	// VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
	// VK_POLYGON_MODE_LINE : polygon edges are drawn as lines
	// VK_POLYGON_MODE_POINT : polygon vertices are drawn as points
	// Using any mode other than fill requires enabling a GPU feature.
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	// Any line thicker than 1.0f requires you to enable the wideLines GPU feature.
	rasterizer.lineWidth = 1.0f;

	// A whole buttload of configurations
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	// Here changes which faces to show of triangles
	// Can be:
	// - VK_FRONT_FACE_CLOCKWISE
	// - VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasClamp = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// MultiSampling
	// It requires enabling a GPU Feature in logical device creation.
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = msaaSamples;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// Depth and stencil testing
	// Not used right now so only a nullptr will be passed

	// Color Blending struct describing the operation
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// // Pseudo Code explaining the above struct
	//if (blendEnable) {
	//	finalColor.rgb = (srcColorBlendFactor * newColor.rgb)
	//		< colorBlendOp > (dstColorBlendFactor * oldColor.rgb);
	//	finalColor.a = (srcAlphaBlendFactor * newColor.a) < alphaBlendOp >
	//		(dstAlphaBlendFactor * oldColor.a);
	//}
	//else {
	//	finalColor = newColor;
	//}
	//finalColor = finalColor & colorWriteMask;

	// Alpha Blending
	//finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
	//finalColor.a = newAlpha.a;
	// The struct that acomplishes it
	//colorBlendAttachment.blendEnable = VK_TRUE;
	//colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	//colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	//colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	//colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// Struct to gather all color blend for all of the framebuffers
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional


	// Dynamic States
	// Some states can be changed without recreating the pipeline like viewport, linewidth and blend constants
	// The dynamic states can be created like this.
	//VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_LINE_WIDTH};
	//VkPipelineDynamicStateCreateInfo dynamicState = {};
	//dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	//dynamicState.dynamicStateCount = 2;
	//dynamicState.pDynamicStates = dynamicStates;

	// If defined, the set values will be ignored and must be specified at drawing time.

	// Uniform values in shaders are similar to dynamics states. Can be changed at drawing time to alter shaders
	// without recreating them. Most commonly used for transformation matrix and texture samplers.
	// Can be specified at the pipeline creation with a VkPipelineLayout object. This is required with or without using uniform values.
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	// Here there is a choice for the Allocator function
	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	// Shaders
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	// Fixed Functions
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	// A depth stencil state must always be specified if the render pass contains a depth stencil attachment
	pipelineInfo.pDepthStencilState = &depthStencil; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	// Layout
	pipelineInfo.layout = pipelineLayout;
	// Render Pass
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	// Vulkan allows you to create a new graphics pipeline by deriving from an existing pipeline.
	// These values are only used if the VK_PIPELINE_CREATE_DERIVATIVE_BIT flag
	// is also specified in the flags field of VkGraphicsPipelineCreateInfo.
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	// Multiple graphics pipeline can be created at the sametime;
	// Here there is a choice for the Allocator functio
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline");
	}

	// Here there is a choice for the Allocator function
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

std::vector<VkFramebuffer> VulkanEngine::createFramebuffers(const VkRenderPass renderPass, const VkE_SwapChain& swapChain, VkImageView colorAttachment, VkImageView depthAttachment) {
	std::vector<VkFramebuffer> frameBuffers(swapChain.imageViews.size(),VK_NULL_HANDLE);
	std::vector<VkImageView> attachments;
	if (colorAttachment) attachments.push_back(colorAttachment);
	if (depthAttachment) attachments.push_back(depthAttachment);

	attachments.push_back(VK_NULL_HANDLE);
	uint32_t attachmentSize = static_cast<uint32_t> (attachments.size());

	for (size_t i = 0; i < swapChain.imageViews.size(); i++) {
		attachments[attachmentSize - 1] = swapChain.imageViews[i];

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = attachmentSize;
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChain.extent.width;
		framebufferInfo.height = swapChain.extent.height;
		framebufferInfo.layers = 1;

		// Here there is a choice for the Allocator function
		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
	return frameBuffers;
}

void VulkanEngine::createCommandPool() {

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = graphicsFamily;
	// Command Pool Flags
	//• VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new 
	//	commands very often(may change memory allocation behavior)
	//• VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : Allow command buffers to be rerecorded 
	//	individually, without this flag they all have to be reset together
	poolInfo.flags = 0; // Optional

	// Here there is a choice for the Allocator function
	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}

	// If transfer family and graphics family are the same use the same command pool
	if (graphicsFamily == transferFamily)
		transientcommandPool = commandPool;
	else {
		// Transfer Challenge & Transient Command Pool Challenge:
		// Create a transient pool for short lived command buffers for memory allocation optimizations.
		VkCommandPoolCreateInfo transientpoolInfo = {};
		transientpoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		transientpoolInfo.queueFamilyIndex = transferFamily;
		transientpoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		// Here there is a choice for the Allocator function
		if (vkCreateCommandPool(device, &transientpoolInfo, nullptr, &transientcommandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create transient command pool!");
		}
	}
}

std::vector<VkCommandBuffer> VulkanEngine::createCommandBuffers(const VkCommandPool commandPool,uint32_t buffersCount) {
	
	std::vector<VkCommandBuffer> commandBuffers(buffersCount, VK_NULL_HANDLE);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	//• VK_COMMAND_BUFFER_LEVEL_PRIMARY : Can be submitted to a queue for execution, but cannot be called from other command buffers.
	//• VK_COMMAND_BUFFER_LEVEL_SECONDARY : Cannot be submitted directly, but can be called from primary command buffers.
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
	return commandBuffers;
}

void VulkanEngine::createSyncObjects() {
	// Create synchronization objects to control the flow of work both on gpu and from cpu to gpu.
	// The semaphores tell the gpu when to wait to start the action. 
	// The fences control when the cpu can submit more work to the gpu based on wether the gpu is done with the frame selected.
	imageAvailableSemaphore.resize(MAX_FRAME_IN_FLIGHT);
	renderFinishedSemaphore.resize(MAX_FRAME_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAME_IN_FLIGHT);
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
		// Here there is a choice for the Allocator function
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

uint32_t VulkanEngine::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) { // I don't get this part
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanEngine::createIndexBuffer(std::vector<uint32_t> indices) {
	indexCount = static_cast<uint32_t>(indices.size());
	VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer,
		indexBufferMemory);
	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}



void VulkanEngine::createDescriptorPool() {
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	// Pool for uniform buffer
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(mainSwapChain.imageCount * MIRROR_DESCRIPTOR_SET_COUNT);

	// Pool for combined image sampler
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(mainSwapChain.imageCount * MAX_SAMPLED_IMAGES * MIRROR_DESCRIPTOR_SET_COUNT);

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(mainSwapChain.imageCount * MIRROR_DESCRIPTOR_SET_COUNT);
	//The structure has an optional flag similar to command pools that determines if
	//	individual descriptor sets can be freed or not: VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT.
	//	We’re not going to touch the descriptor set after creating it, so we don’t need
	//	this flag.You can leave flags to its default value of 0.
	poolInfo.flags = 0;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void VulkanEngine::createDescriptorSets() {
	std::vector<VkDescriptorSetLayout> layouts(mainSwapChain.imageCount, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();

	for (int i = 0; i < MIRROR_DESCRIPTOR_SET_COUNT; i++) {
		descriptorSets[i].resize(layouts.size());

		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets[i].data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
	}
}

void VulkanEngine::createColorResources() {
	VkFormat colorFormat = swapChainImageFormat;

	createImage(swapChainExtent.width, swapChainExtent.height,
		1, msaaSamples,
		colorFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		colorImage, colorImageMemory);

	colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

VkFormat VulkanEngine::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	// The support of a format depends on the tiling mode and usage

	for (VkFormat format : candidates) {
		/*
		• linearTilingFeatures : Use cases that are supported with linear tiling
		• optimalTilingFeatures : Use cases that are supported with optimal tiling
		• bufferFeatures : Use cases that are supported for buffers
		*/
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

VkFormat VulkanEngine::findDepthFormat() {
	/*
		• VK_FORMAT_D32_SFLOAT : 32 - bit float for depth
		• VK_FORMAT_D32_SFLOAT_S8_UINT : 32 - bit signed float for depth and 8 bit stencil component
		• VK_FORMAT_D24_UNORM_S8_UINT : 24 - bit float for depth and 8 bit stencil component
	*/
	return findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

void VulkanEngine::createDepthResources() {


	VkFormat depthFormat = findDepthFormat();

	createImage(swapChainExtent.width, swapChainExtent.height,
		1, msaaSamples,
		depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage,
		depthImageMemory);
	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	transitionImageLayout(depthImage, depthFormat,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		1);
}

void VulkanEngine::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	// Create a buffer handle with the specified memory needed and usage
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;

	// Transfer Challenge:
	// Use a different queue family and queue specifically for transfer operations.
	// If we're  using the buffer on different queue families we can either choose sharing mode exclusive
	// and resolve ownership issues or choose sharing mode concurrent where we are required to specify 
	// the different queue families that will be using the buffer. The former option offers better performance.
	// In this function, I differentiate between using different and single queue families by specifying 
	// th VK_BUFFER_USAGE_TRANSFER_DST_BIT flag or not, respectively.
	if ((usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) && (graphicsFamily != transferFamily)) {
		uint32_t queueFamilyIndices[] = { graphicsFamily,transferFamily };
		bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferInfo.queueFamilyIndexCount = 2;
		bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	bufferInfo.flags = 0; // The flags parameter is used to configure sparse buffer memory
	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}
	// Query the created buffer for the memory requirements the physical device specifies.
	// The memory requirements struct consists of:
	// • size : The size of the required amount of memory in bytes, may differ from bufferInfo.size.
	// • alignment : The offset in bytes where the buffer begins in the allocated region of memory, depends on bufferInfo.usage and bufferInfo.flags.
	// • memoryTypeBits : Bit field of the memory types that are suitable for the buffer.
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	// Create a struct to hold the memory allocation requirements
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	// This is the memory index of the physical device available memory types
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
	// Finally allocate memory
	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	// Now bind the memory to buffer created before
	// the fourth parameter is the offset within the region of memory.Since this memory is allocated specifically
	// for this the vertex buffer, the offset is simply 0. If the offset is non - zero, then it is required to be divisible by memRequirements.alignment
	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanEngine::mapBufferMemory(VkDeviceMemory bufferMemory, void* data, VkDeviceSize datalen) {
	// To copy the vertices data to the allocated memory we need query a pointer to copy
	void* bufferData;
	// Map the memory allocated to a CPU accessible memory
	vkMapMemory(device, bufferMemory, 0, datalen, 0, &bufferData);
	// Copy the vertices data
	memcpy(bufferData, data, (size_t)datalen);
	vkUnmapMemory(device, bufferMemory);
}

void VulkanEngine::destroyBufferBundle(BufferBundle buffer) {
	vkDestroyBuffer(device, buffer.buffer, nullptr);
	vkFreeMemory(device, buffer.memory, nullptr);
};

void VulkanEngine::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	// To copy between buffer we must submit a command. We're going to create a
	// temporary command buffer to do so. We can use a separate command pool with the
	// VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag when creating it.
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// How to copy contents between buffers.
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size; // It is not possible to specify VK_WHOLE_SIZE here, unlike the vkMapMemory command
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
}

void VulkanEngine::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0; // Padding between rows
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer VulkanEngine::beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = transientcommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;

}

void VulkanEngine::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	// Unlike the draw commands, there are no events we need to wait on this time.
	// We just want to execute the transfer on the buffers immediately.There are
	// again two possible ways to wait on this transfer to complete.We could use a
	// fence and wait with vkWaitForFences, or simply wait for the transfer queue
	// to become idle with vkQueueWaitIdle.A fence would allow you to schedule
	// multiple transfers simultaneously and wait for all of them complete, instead
	// of executing one at a time.That may give the driver more opportunities to
	// optimize.
	vkQueueWaitIdle(transferQueue);
	// Memory that is bound to a buffer object may be freed once
	// the buffer is no longer used, so let’s free it after the buffer has been destroyed
	vkFreeCommandBuffers(device, transientcommandPool, 1, &commandBuffer);
}

void VulkanEngine::createImage(uint32_t width, uint32_t height,
	uint32_t mipLevels, VkSampleCountFlagBits numSamples,
	VkFormat format,
	VkImageTiling tiling, VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties, VkImage& image,
	VkDeviceMemory& imageMemory) {
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	/*
	Vulkan supports many possible image formats, but we should use the same
	format for the texels as the pixels in the buffer, otherwise the copy operation
	will fail.
	*/
	imageInfo.format = format;
	/*
	The tiling field can have one of two values:
	• VK_IMAGE_TILING_LINEAR: Texels are laid out in row-major order like our pixels array
	• VK_IMAGE_TILING_OPTIMAL: Texels are laid out in an implementation defined	order for optimal access
	*/
	imageInfo.tiling = tiling;
	/*
	* There are only two possible values for the initialLayout of an image:
	• VK_IMAGE_LAYOUT_UNDEFINED: Not usable by the GPU and the very first transition will discard the texels.
	• VK_IMAGE_LAYOUT_PREINITIALIZED: Not usable by the GPU, but the first transition will preserve the texels.
	*/
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	imageInfo.usage = usage;
	// Image will only be used by one queue family: the graphics queue
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	imageInfo.samples = numSamples;
	imageInfo.flags = 0; // Optional

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}
	vkBindImageMemory(device, image, imageMemory, 0);
}

VkImageView VulkanEngine::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
	VkImageViewCreateInfo createInfo = {  };
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = mipLevels;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	// Here there is a choice for the Allocator function
	if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create image views!");
	}
	return imageView;
}

void VulkanEngine::createImageSampler(VkSampler& sampler, uint32_t mipLevels) {
	// Sampler is a separate object that can be used on any image and doesn't reference the image anywhere
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	/*
	* VK_FILTER_LINEAR
	* VK_FILTER_NEAREST
	*/
	// Filter for oversampling 
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	// Filter for undersampling
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	/*
	Address mode: What happens when reading texels outside the picture
	• VK_SAMPLER_ADDRESS_MODE_REPEAT: Repeat the texture when going beyond the image dimensions.
	• VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Like repeat, but inverts
		the coordinates to mirror the image when going beyond the dimensions.
	• VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: Take the color of the edge
		closest to the coordinate beyond the image dimensions.
	• VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: Like clamp to edge,
		but instead uses the edge opposite to the closest edge.
	• VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: Return a solid color
		when sampling beyond the dimensions of the image.
	*/
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	// Unless for performance reasons always enable anisotropic filter
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	// Color when address mode is clamp to border.
	// Can be Black, white or transparent
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	// Whether to normalize the coordinates to [0,1]
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	// used for percentage-closer filtering on shadow maps
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	// Filtering for mipmapping
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0;
	samplerInfo.minLod = 0; // minimum level of detail to choose mip levels
	samplerInfo.maxLod = static_cast<float>(mipLevels); // maximum level of detail to choose mip levels


	if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void VulkanEngine::createSampledImage(SampledImage& image, int cols, int rows, int elemSize, char* imageData,uint32_t mipLvls, VkSampleCountFlagBits numsamples) {
	VkDeviceSize imageSize = rows*cols * elemSize;


	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
		stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, imageData, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	createImage(
		cols,
		rows,
		mipLvls,
		numsamples,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		(mipLvls>1 ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0) | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		image.image,
		image.memory);

	if (mipLvls > 1) {
		transitionImageLayout(image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLvls);
		copyBufferToImage(stagingBuffer, image.image, static_cast<uint32_t>(cols), static_cast<uint32_t>(rows));
		generateMipmaps(image.image, VK_FORMAT_R8G8B8A8_SRGB, cols, rows, mipLvls);
	}
	else {
		transitionImageLayout(image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLvls);
		copyBufferToImage(stagingBuffer, image.image, static_cast<uint32_t>(cols), static_cast<uint32_t>(rows));
		transitionImageLayout(image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLvls);
	}

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	image.view = createImageView(image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLvls);

	createImageSampler(image.sampler, mipLvls);
}

void VulkanEngine::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {

	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	if (graphicsFamily == transferFamily) {
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	}
	else {
		barrier.srcQueueFamilyIndex = transferFamily;
		barrier.dstQueueFamilyIndex = graphicsFamily;
	}
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (hasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}


	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
	endSingleTimeCommands(commandBuffer);
}

void VulkanEngine::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {

	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

	/*
	There are two alternatives in this case.You could implement a function that
	searches common texture image formats for one that does support linear blitting,
	or you could implement the mipmap generation in software with a library like
	stb_image_resize.Each mip level can then be loaded into the image in the
	same way that you loaded the original image.
	It should be noted that it is uncommon in practice to generate the mipmap
	levels at runtime anyway.Usually they are pregenerated and stored in the
	texture file alongside the base level to improve loading speed.Implementing
	resizing in software and loading multiple levels from a file is left as an exercise
	to the reader.
	*/
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}
	// This function assumes all the mipLevels are in layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++) {
		// Transition the source image of the blit to layout VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		// Blit the same image between different miplevels
		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	endSingleTimeCommands(commandBuffer);

}

VkSampleCountFlagBits VulkanEngine::getMaxUsableSampleCount() {
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
		physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
	if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
	if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
	if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
	if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
	if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;
	return VK_SAMPLE_COUNT_1_BIT;
}

void VulkanEngine::freeDescriptorSet(VkDescriptorPool pool,VkDescriptorSet& set) {
	vkFreeDescriptorSets(device, pool, 1, &set);
	set = VK_NULL_HANDLE;
};

//void VulkanEngine::createTextureImage() {
//	int texWidth, texHeight, texChannels;
//
//	cv::Mat matImage = loadImage(TEXTURE_PATH);
//	VkDeviceSize imageSize = matImage.total() * matImage.elemSize();
//
//	texWidth = matImage.cols;
//	texHeight = matImage.rows;
//	texChannels = 4;
//
//	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
//	VkBuffer stagingBuffer;
//	VkDeviceMemory stagingBufferMemory;
//
//	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
//		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
//		stagingBufferMemory);
//
//	void* data;
//	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
//	memcpy(data, matImage.data, static_cast<size_t>(imageSize));
//	vkUnmapMemory(device, stagingBufferMemory);
//
//	matImage.release();
//
//	createImage(
//		texWidth,
//		texHeight,
//		mipLevels,
//		VK_SAMPLE_COUNT_1_BIT,
//		VK_FORMAT_R8G8B8A8_SRGB,
//		VK_IMAGE_TILING_OPTIMAL,
//		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//		textureImage,
//		textureImageMemory);
//
//	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
//	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
//	//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
//
//	//transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
//	//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//	//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
//	generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
//
//	vkDestroyBuffer(device, stagingBuffer, nullptr);
//	vkFreeMemory(device, stagingBufferMemory, nullptr);
//}
//
//
//void VulkanEngine::createTextureImageView() {
//	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
//}
//
//void VulkanEngine::createTextureSampler() {
//	// Sampler is a separate object that can be used on any image and doesn't reference the image anywhere
//	VkSamplerCreateInfo samplerInfo = {};
//	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//	/*
//	* VK_FILTER_LINEAR
//	* VK_FILTER_NEAREST
//	*/
//	// Filter for oversampling 
//	samplerInfo.magFilter = VK_FILTER_LINEAR;
//	// Filter for undersampling
//	samplerInfo.minFilter = VK_FILTER_LINEAR;
//	/*
//	Address mode: What happens when reading texels outside the picture
//	• VK_SAMPLER_ADDRESS_MODE_REPEAT: Repeat the texture when going beyond the image dimensions.
//	• VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Like repeat, but inverts
//		the coordinates to mirror the image when going beyond the dimensions.
//	• VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: Take the color of the edge
//		closest to the coordinate beyond the image dimensions.
//	• VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: Like clamp to edge,
//		but instead uses the edge opposite to the closest edge.
//	• VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: Return a solid color
//		when sampling beyond the dimensions of the image.
//	*/
//	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//	// Unless for performance reasons always enable anisotropic filter
//	samplerInfo.anisotropyEnable = VK_TRUE;
//	samplerInfo.maxAnisotropy = 16;
//	// Color when address mode is clamp to border.
//	// Can be Black, white or transparent
//	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
//	// Whether to normalize the coordinates to [0,1]
//	samplerInfo.unnormalizedCoordinates = VK_FALSE;
//	// used for percentage-closer filtering on shadow maps
//	samplerInfo.compareEnable = VK_FALSE;
//	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
//	// Filtering for mipmapping
//	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//	samplerInfo.mipLodBias = 0;
//	samplerInfo.minLod = 0; // minimum level of detail to choose mip levels
//	samplerInfo.maxLod = static_cast<float>(mipLevels); // maximum level of detail to choose mip levels
//
//	if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
//		throw std::runtime_error("failed to create texture sampler!");
//	}
//}