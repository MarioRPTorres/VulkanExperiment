#include "vulkan_engine.h"
#include "vulkan_vertices.h"
#include <chrono>
#include <map>


// glsl_shader.vert, compiled with:
// # python ./groupHexadecimal.py './shader.vert'
/*
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec2 fragColor;

void main(){
	gl_Position = vec4(inPosition,0.0, 1.0);
	fragColor = inPosition*0.5+0.5;
}
*/

static shaderCode32 vert = {
0x07230203, 0x00010000, 0x000d000a, 0x00000022, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
0x0008000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x00000012, 0x0000001c,
0x00030003, 0x00000002, 0x000001c2, 0x00090004, 0x415f4c47, 0x735f4252, 0x72617065, 0x5f657461,
0x64616873, 0x6f5f7265, 0x63656a62, 0x00007374, 0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45,
0x74735f70, 0x5f656c79, 0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47,
0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669, 0x00040005, 0x00000004,
0x6e69616d, 0x00000000, 0x00060005, 0x0000000b, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000,
0x00060006, 0x0000000b, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000000b,
0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b, 0x00000002,
0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e, 0x00070006, 0x0000000b, 0x00000003, 0x435f6c67,
0x446c6c75, 0x61747369, 0x0065636e, 0x00030005, 0x0000000d, 0x00000000, 0x00050005, 0x00000012,
0x6f506e69, 0x69746973, 0x00006e6f, 0x00050005, 0x0000001c, 0x67617266, 0x6f6c6f43, 0x00000072,
0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000, 0x00050048, 0x0000000b, 0x00000001,
0x0000000b, 0x00000001, 0x00050048, 0x0000000b, 0x00000002, 0x0000000b, 0x00000003, 0x00050048,
0x0000000b, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000000b, 0x00000002, 0x00040047,
0x00000012, 0x0000001e, 0x00000000, 0x00040047, 0x0000001c, 0x0000001e, 0x00000000, 0x00020013,
0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017,
0x00000007, 0x00000006, 0x00000004, 0x00040015, 0x00000008, 0x00000020, 0x00000000, 0x0004002b,
0x00000008, 0x00000009, 0x00000001, 0x0004001c, 0x0000000a, 0x00000006, 0x00000009, 0x0006001e,
0x0000000b, 0x00000007, 0x00000006, 0x0000000a, 0x0000000a, 0x00040020, 0x0000000c, 0x00000003,
0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d, 0x00000003, 0x00040015, 0x0000000e, 0x00000020,
0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000, 0x00040017, 0x00000010, 0x00000006,
0x00000002, 0x00040020, 0x00000011, 0x00000001, 0x00000010, 0x0004003b, 0x00000011, 0x00000012,
0x00000001, 0x0004002b, 0x00000006, 0x00000014, 0x00000000, 0x0004002b, 0x00000006, 0x00000015,
0x3f800000, 0x00040020, 0x00000019, 0x00000003, 0x00000007, 0x00040020, 0x0000001b, 0x00000003,
0x00000010, 0x0004003b, 0x0000001b, 0x0000001c, 0x00000003, 0x0004002b, 0x00000006, 0x0000001e,
0x3f000000, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005,
0x0004003d, 0x00000010, 0x00000013, 0x00000012, 0x00050051, 0x00000006, 0x00000016, 0x00000013,
0x00000000, 0x00050051, 0x00000006, 0x00000017, 0x00000013, 0x00000001, 0x00070050, 0x00000007,
0x00000018, 0x00000016, 0x00000017, 0x00000014, 0x00000015, 0x00050041, 0x00000019, 0x0000001a,
0x0000000d, 0x0000000f, 0x0003003e, 0x0000001a, 0x00000018, 0x0004003d, 0x00000010, 0x0000001d,
0x00000012, 0x0005008e, 0x00000010, 0x0000001f, 0x0000001d, 0x0000001e, 0x00050050, 0x00000010,
0x00000020, 0x0000001e, 0x0000001e, 0x00050081, 0x00000010, 0x00000021, 0x0000001f, 0x00000020,
0x0003003e, 0x0000001c, 0x00000021, 0x000100fd, 0x00010038
};


// glsl_shader.frag, compiled with:
// # python ./groupHexadecimal.py './shader.frag'
/*
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fragColor;

layout(location = 0) out vec4 outColor;


void main() {
	outColor = vec4(fragColor,0.5, 1.0);
}
*/

static shaderCode32 frag = {
0x07230203, 0x00010000, 0x000d000a, 0x00000013, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
0x0007000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000c, 0x00030010,
0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00090004, 0x415f4c47, 0x735f4252,
0x72617065, 0x5f657461, 0x64616873, 0x6f5f7265, 0x63656a62, 0x00007374, 0x000a0004, 0x475f4c47,
0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79, 0x656e696c, 0x7269645f, 0x69746365, 0x00006576,
0x00080004, 0x475f4c47, 0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669,
0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009, 0x4374756f, 0x726f6c6f,
0x00000000, 0x00050005, 0x0000000c, 0x67617266, 0x6f6c6f43, 0x00000072, 0x00040047, 0x00000009,
0x0000001e, 0x00000000, 0x00040047, 0x0000000c, 0x0000001e, 0x00000000, 0x00020013, 0x00000002,
0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007,
0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b, 0x00000008,
0x00000009, 0x00000003, 0x00040017, 0x0000000a, 0x00000006, 0x00000002, 0x00040020, 0x0000000b,
0x00000001, 0x0000000a, 0x0004003b, 0x0000000b, 0x0000000c, 0x00000001, 0x0004002b, 0x00000006,
0x0000000e, 0x3f000000, 0x0004002b, 0x00000006, 0x0000000f, 0x3f800000, 0x00050036, 0x00000002,
0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x0000000a, 0x0000000d,
0x0000000c, 0x00050051, 0x00000006, 0x00000010, 0x0000000d, 0x00000000, 0x00050051, 0x00000006,
0x00000011, 0x0000000d, 0x00000001, 0x00070050, 0x00000007, 0x00000012, 0x00000010, 0x00000011,
0x0000000e, 0x0000000f, 0x0003003e, 0x00000009, 0x00000012, 0x000100fd, 0x00010038
};


#define WINDOWS_COUNT 3

#define QUDI(x) #x
#define STRING(x) QUDI(x)

/*
Future Feature:
• Push constants
• Instanced rendering
• Dynamic uniforms
• Separate images and sampler descriptors
• Pipeline cache
• Multi-threaded command buffer generation
• Multiple subpasses
• Compute shaders
*/

std::array< P2Vertex, 3> vertices = {
	{
		{ {-1.0f,-1.0f} },
		{ {-1.0f,1.0f} },
		{ {1.0f,-1.0f} },
	}
};

std::array<uint32_t, 3> indices = { 0,1,2 };

static std::vector<char>readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read((char*)buffer.data(), fileSize);

	file.close();

	return buffer;
}


class MultipleWindowsApplication :public VulkanWindow {
	using VulkanWindow::VulkanWindow;
public:
	bool running = false;
	float WIDTH = 0;
	float HEIGHT = 0;
	glm::vec3 clearColor = { 0,0,0 };
	MultipleWindowsApplication(VulkanEngine* vk, float width, float height, glm::vec3 clearColor) : VulkanWindow(vk), WIDTH(width), HEIGHT(height), clearColor(clearColor) {};
	void init() {
		msaaSamples = VK_SAMPLE_COUNT_1_BIT;
		mipLevels = 3;
		initWindow();
		initVulkan();
		running = true;
	}
	VkE_Buffer vertexBuffer;
	VkE_Buffer indexBuffer;
	
	VkShaderModule vertShaderModule = VK_NULL_HANDLE;
	VkShaderModule fragShaderModule = VK_NULL_HANDLE;

	uint32_t indexCount = 0;

	void initWindow() {

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tells GLFW to not create an OpenGL context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);	// Disable resizable window

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		// Sets a pointer to the application inside window handle
		glfwSetWindowUserPointer(window, this);
		// Sets a callback for size change
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		// Our HelloTriangleApplication object
		auto app = reinterpret_cast<MultipleWindowsApplication*>(glfwGetWindowUserPointer(window));
		// This flag is used for explicit resize because it is not garanteed that the driver will send a out-of-date error when the window is resized.
		app->framebufferResized = true;
	}

	void initVulkan() {
		vk->createSurface(window, surface);
		if (vk->device == VK_NULL_HANDLE) {
			vk->pickPhysicalDevice(surface);
			vk->createLogicalDevice();
		}

		auto vkbd = vk->getBackEndData();
		graphicsQueue = vkbd.graphicsQueue;
		presentQueue = vkbd.presentQueue;

		vertShaderModule = vk->createShaderModule(vert);
		fragShaderModule = vk->createShaderModule(frag);
		vk->createCommandPool(commandPool, vkbd.graphicsQueueFamily, 0);
		// If transfer family and graphics family are the same use the same command pool
		VkCommandPool transientCommandPool;
		if (vkbd.transientCommandPool == VK_NULL_HANDLE) {
			vk->createCommandPool(transientCommandPool, vkbd.transferQueueFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
			vk->setTransientCommandPool(transientCommandPool);
		}
		vk->createVertexBuffer(vertices.data(), vertices.size() * sizeof(vertices[0]), vertexBuffer);
		vk->createIndexBuffer(indices.data(), indices.size() * sizeof(indices[0]), indexBuffer);
		indexCount = static_cast<uint32_t>(indices.size());

		createSwapChainObjects();
	}

	int mainLoop() {
		// Event Handler
		if (glfwWindowShouldClose(window)) {
			running = false;
			cleanup();
			return 0;
		}
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		if (width == 0 || height == 0) return 1;
		
		// Check the special case where the window is minimized and the framebuffer size is zero

		drawFrame();
		return 1;
	}

	void cleanup() {
		std::vector<VkFence>& inFlightFences = syncObjects.inFlightFences;
		vkWaitForFences(vk->device, inFlightFences.size(), inFlightFences.data(), VK_TRUE, UINT64_MAX);

		cleanupSwapChain();

		vkDestroyShaderModule(vk->device, vertShaderModule, nullptr); vertShaderModule = VK_NULL_HANDLE;
		vkDestroyShaderModule(vk->device, fragShaderModule, nullptr); fragShaderModule = VK_NULL_HANDLE;

		// Here there is a choice for the Allocator function
		vk->destroyBufferBundle(vertexBuffer);
		vk->destroyBufferBundle(indexBuffer);

		// Here there is a choice for the Allocator function
		vkDestroyCommandPool(vk->device, commandPool, nullptr);

		vk->destroySurface(surface);

		glfwDestroyWindow(window);	// Cleanup Window Resources

	}

	void cleanupSwapChain() {
		vk->cleanupSyncObjects(syncObjects);
		for (size_t i = 0; i < frameBuffers.size(); i++) {
			vkDestroyFramebuffer(vk->device, frameBuffers[i], nullptr);
		}
		vkFreeCommandBuffers(vk->device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkDestroyPipeline(vk->device, pipeline, nullptr);
		vkDestroyPipelineLayout(vk->device, pipelineLayout, nullptr);
		vkDestroyRenderPass(vk->device, renderPass, nullptr);


		for (size_t i = 0; i < sc.imageViews.size(); i++) {
			vkDestroyImageView(vk->device, sc.imageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(vk->device, sc.swapChain, nullptr);

	}

	void createSwapChainObjects() {
		VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		// Recreate the swapchain
		vk->createSwapChain(surface, sc,extent);
		vk->createSwapChainImageViews(sc.images, sc.format, sc.imageViews);
		// The render pass depends on the format of the swap chain. It is rare that the format changes but to be sure
		vk->createRenderPass(renderPass, sc.format, msaaSamples, true, true, false, true);
		vk->createGraphicsPipeline(pipeline, pipelineLayout, renderPass, vertShaderModule, fragShaderModule, P2Vertex::getDescriptions(), nullptr, VK_NULL_HANDLE, sc.extent, msaaSamples);
		frameBuffers = vk->createFramebuffers(renderPass, sc);
		commandBuffers = vk->createCommandBuffers(commandPool, frameBuffers.size(), true);
		vk->createSyncObjects(syncObjects, sc.imageCount);
		writeCommandBuffers();
	}

	void recreateSwapChain() {
		swapChainOutdated = false;
		// Swap chain recreation for events like window resizing

		// First wait for vk->device to be idle so that all resources are free from use

		std::vector<VkFence>& inFlightFences = syncObjects.inFlightFences;
		vkWaitForFences(vk->device, inFlightFences.size(), inFlightFences.data(), VK_TRUE, UINT64_MAX);

		cleanupSwapChain();
		createSwapChainObjects();
	}



	void drawFrame() {
		// The vkWaitForFences function takes an array of fences and waits for either
		// any or all of them to be signaled before returning.The VK_TRUE we pass here
		// indicates that we want to wait for all fences, but in the case of a single one it
		// obviously doesn’t matter.Just like vkAcquireNextImageKHR this function also
		// takes a timeout.Unlike the semaphores, we manually need to restore the fence
		// to the unsignaled state by resetting it with the vkResetFences call.
		vkWaitForFences(vk->device, 1, &syncObjects.inFlightFences[inFlightFrameIndex], VK_TRUE, UINT64_MAX);

		// The drawFrame function perform three operations:
		//	• Acquire an image from the swap chain
		//	• Execute the command buffer with that image as attachment in the framebuffer
		//	• Return the image to the swap chain for presentation
		// However the functions that acomplish this return before the operation is actually finished and each operation depends
		// on the previous one being finished. To syncronize them we need semaphores which are use for syncronizing operations within
		// or across command queues. If we want to syncronize calls between the application and the rendering operations, we should use fences instead.



		// vkAcquireNextImageKHR arguments
		//• Logical vk->device
		//• Swap Chain to get the image
		//• Timeout in nanoseconds for the image to become available. The max value of 64 bit unsigned integer disables it
		//• Semaphore to signal when finished
		//• Fence to signal when finished
		//• Output index of image in the swap chain
		// Frames are now set to be concurrently with multiple semaphores.
		// vkAcquireNextImageKHR possible outpus
		//• VK_ERROR_OUT_OF_DATE_KHR : The swap chain has become incompatible with the surface and can no longer be used for rendering.Usually happens
		// after a window resize.
		//• VK_SUBOPTIMAL_KHR : The swap chain can still be used to successfully present to the surface, but the surface properties are no longer matched
		// exactly.
		VkResult result = vkAcquireNextImageKHR(vk->device, sc.swapChain, UINT64_MAX, syncObjects.imageAvailableSemaphore[inFlightFrameIndex], VK_NULL_HANDLE, &imageIndex);

		// Using the result we can check if the swapchain is out of data and if so we need to recreate it
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			swapChainOutdated = true;
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { // Since we already adquired an image we can still draw even if its suboptimal
			throw std::runtime_error("failed to acquire swap chain image!");
		}


		// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (syncObjects.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(vk->device, 1, &syncObjects.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}
		// Mark the image as now being in use by this frame
		syncObjects.imagesInFlight[imageIndex] = syncObjects.inFlightFences[inFlightFrameIndex];


		std::vector<VkCommandBuffer> submitCommmandBuffers = { commandBuffers[imageIndex] };
		VkResult err;


		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//We want to wait with writing colors to the image until it’s available, so we’re specifying the stage of
		//the graphics pipeline that writes to the color attachment. That means that theoretically the 
		//implementation can already start executing our vertex shader and such while the image is not yet available.
		//Each entry in the waitStages array corresponds to the semaphore with the same index in pWaitSemaphores.
		VkSemaphore waitSemaphores[] = { syncObjects.imageAvailableSemaphore[inFlightFrameIndex] };

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		// Commands to execute
		submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommmandBuffers.size());
		submitInfo.pCommandBuffers = submitCommmandBuffers.data();

		VkSemaphore signalSemaphores[] = { syncObjects.renderFinishedSemaphore[inFlightFrameIndex] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(vk->device, 1, &syncObjects.inFlightFences[inFlightFrameIndex]);

		// Here we submit the command to draw the triangle.
		// We also use the current Frame Fence to signal when the command buffer finishes executing, this way we know when a frame is finished
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, syncObjects.inFlightFences[inFlightFrameIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw comand buffer!");
		}


		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		//The first two parameters specify which semaphores to wait on before presentation
		//can happen, just like VkSubmitInfo.
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &syncObjects.renderFinishedSemaphore[inFlightFrameIndex];

		VkSwapchainKHR swapChains[] = { sc.swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		// vkQueuePresentKHR possible outpus
		//• VK_ERROR_OUT_OF_DATE_KHR : The swap chain has become incompatible with the surface and can no longer be used for rendering.Usually happens
		// after a window resize.
		//• VK_SUBOPTIMAL_KHR : The swap chain can still be used to successfully present to the surface, but the surface properties are no longer matched
		// exactly.
		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		// Using the result we can check if the swapchain is out of data and if so we need to recreate it
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		// Because we are not waiting for the presentation to finish and continuosly running drawFrame
		// The cpu might be submitting more work than what the gpu can handle. Also the imageAvailableSemaphore
		// and renderFinishedSemaphore semaphores will be used with multiple frames at a time. If we use vkQueueWaitIdle
		// we syncronize in a rudimentary way but the GPU will have to only process one frame at the time which is not
		// efficient.
		//vkQueueWaitIdle(presentQueue);
		// EDIT: This was the old solution. The syncronization is now done with multiple semaphores and fences to keep the gpu syncronize, limit the work submitted and memory usage
		// as well as making sure no swapchain images are reused.

		// Syncronization is only done with GPU-GPU not CPU-GPU so more work can still be submitted. Gpu only waits for the previous operation to be ready.
		inFlightFrameIndex = (inFlightFrameIndex + 1) % MAX_FRAME_IN_FLIGHT;
	}


	void writeCommandBuffers() {

		vkResetCommandPool(vk->device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		for (size_t i = 0; i < commandBuffers.size(); i++) {

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			//• VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : The command buffer will be rerecorded right after 
			//	executing it once.
			//• VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : This is a secondary command buffer that will be 
			//	entirely within a single render pass.
			//• VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : The command buffer can be resubmitted while it is also 
			//	already pending execution.
			beginInfo.flags = 0; // OPtional
			//  The pInheritanceInfo parameter is only relevant for secondary command
			//	buffers.It specifies which state to inherit from the calling primary command
			//	buffers.
			beginInfo.pInheritanceInfo = nullptr; // Optional

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = frameBuffers[i];
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = sc.extent;

			std::array<VkClearValue, 1> clearValues = {};
			clearValues[0].color = {clearColor[0],clearColor[1],clearColor[2],1.0f};

			//Note that the order of clearValues should be identical to the order of your attachments in the subpass. 
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			//All of the functions that record commands can be recognized by their vkCmd prefix.

			//• VK_SUBPASS_CONTENTS_INLINE : The render pass commands will be embedded in the primary command 
			//	buffer itself and no secondary command buffers will be executed.
			//• VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : The render pass commands will be executed
			//	from secondary command buffers.
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			//• vertexCount : Even though we don’t have a vertex buffer, we technically
			//	still have 3 vertices to draw.
			//• instanceCount : Used for instanced rendering, use 1 if you’re not doing
			//	that.
			//• firstVertex : Used as an offset into the vertex buffer, defines the lowest
			//	value of gl_VertexIndex.
			//• firstInstance : Used as an offset for instanced rendering, defines the
			//	lowest value of gl_InstanceIndex.
			//vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
			// EDIT: The previous commented was for a hard coded vertices in the shader. The application now allocates a vertex buffer outside
			// and copys the vertices data from the application to the buffer

			VkBuffer vertexBuffers[] = { vertexBuffer.buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
			// We can only have a single index buffer.
			vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

			// Draw Vertex
			//vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
			// Indexed Vertex Draw
			vkCmdDrawIndexed(commandBuffers[i], indexCount, 1, 0, 0, 0);


			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}
};



int main() {
	const int windowsCount = (WINDOWS_COUNT < 1 ? 1 : WINDOWS_COUNT);
	VulkanEngine vk;
	std::vector<MultipleWindowsApplication> apps;
	bool running = false;

	try {

		// Initiates the GLFW library
		glfwInit();
		vk.createInstance();
		vk.setupDebugMessenger();

		for (int i = 0; i < windowsCount; i++) {
			glm::vec3 clearColor = { i == 0 ? 1.0f : 0.0f, i == 1 ? 1.0f : 0.0f, i == 2 ? 1.0f : 0.0f };
			apps.push_back(MultipleWindowsApplication(&vk, 200 * (i + 1), 100 * (i + 1),clearColor));
			apps[i].init();
		}
		running = true;
		while (running) {
			running = false;
			glfwPollEvents();
			for (int i = 0; i < windowsCount; i++) {
				if (apps[i].running) {
					bool r = apps[i].mainLoop();
					running = running || r;
				};
			}
		}
		vkDeviceWaitIdle(vk.device);
		auto vkbd = vk.getBackEndData();
		vkDestroyCommandPool(vk.device, vkbd.transientCommandPool, nullptr);
		vk.shutdownVulkanEngine();
		glfwTerminate();	// Terminate GLFW Library
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;

}