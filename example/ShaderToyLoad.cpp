#include "vulkan_engine.h"
#include "vulkan_vertices.h"
#include <chrono>

//#include <fstream>
#define QUDI(x) #x
#define STRING(x) QUDI(x)


const int WIDTH = 640;
const int HEIGHT = 480;

const char vertexShader[] = "shaderToyV";
const char fragShader[] = "shaderToyF";

struct PushConstants {
	alignas(8)	float resolution[2];
	alignas(4)	float time;
};

static std::vector<char>readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void compileShaders() {
	// TO DO: Add dynamic location of vulkan installation 
	std::string command = STRING(Vulkan_GLSLC_EXECUTABLE);
	
	std::string vertexCommand = ((((command + " ./shaders/") + vertexShader) + ".vert -o ./") + vertexShader) + ".spv";
	std::string fragCommand = ((((command + " ./shaders/") + fragShader) + ".frag -o ./") + fragShader) + ".spv";
	system(vertexCommand.c_str());

	system(fragCommand.c_str());
}

class ShaderToyApplication :protected VulkanEngine {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
private:
	int width = 0;
	int height = 0;
	VkShaderModule vert;
	VkShaderModule frag;
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<VkCommandBuffer> commandBuffers;
	VkSurfaceKHR mainSurface;


	size_t inFlightFrameIndex = 0;
	uint32_t imageIndex;
	bool framebufferResized = false;
	bool swapChainOutdated = false;

	void initWindow() {
		// Initiates the GLFW library
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tells GLFW to not create an OpenGL context
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);	// Disable resizable window

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		// Sets a pointer to the application inside window handle
		glfwSetWindowUserPointer(window, this);
		// Sets a callback for size change
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

		glfwGetFramebufferSize(window, &width, &height);
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		// Our HelloTriangleApplication object
		auto app = reinterpret_cast<ShaderToyApplication*>(glfwGetWindowUserPointer(window));
		// This flag is used for explicit resize because it is not garanteed that the driver will send a out-of-date error when the window is resized.
		app->framebufferResized = true;
	}


	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface(window,mainSurface);
		pickPhysicalDevice(mainSurface);
		createLogicalDevice();
		createSwapChain(mainSurface,mainSwapChain);
		createSwapChainImageViews(mainSwapChain);
		createRenderPass(renderPass,mainSwapChain.format, VK_SAMPLE_COUNT_1_BIT, true, true,false,true);

		std::string vs = "./";
		vs.append(vertexShader);
		vs.append(".spv");
		std::string fs = "./";
		fs.append(fragShader);
		fs.append(".spv");
		shaderCode32 vertCode, fragCode;
		char2shaderCode(readFile(vs), vertCode);
		char2shaderCode(readFile(fs), fragCode);
		vert = createShaderModule(vertCode);
		frag = createShaderModule(fragCode);
		std::vector<VkPushConstantRange> push(1, { VK_SHADER_STAGE_FRAGMENT_BIT,0,sizeof(PushConstants) });
		createGraphicsPipeline(graphicsPipeline, pipelineLayout, renderPass, vert, frag, constants::NullVertexDescriptions, &push, VK_NULL_HANDLE, mainSwapChain.extent, VK_SAMPLE_COUNT_1_BIT);

		createCommandPool(commandPool, graphicsFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		// If transfer family and graphics family are the same use the same command pool
		if (graphicsFamily == transferFamily)
			transientcommandPool = commandPool;
		else {
			// Transfer Challenge & Transient Command Pool Challenge:
			// Create a transient pool for short lived command buffers for memory allocation optimizations.
			createCommandPool(transientcommandPool, transferFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		}
		swapChainFramebuffers = createFramebuffers(renderPass,mainSwapChain);
		commandBuffers = createCommandBuffers(commandPool, swapChainFramebuffers.size(),true);
		createSyncObjects(syncObjects, mainSwapChain.imageCount);
	}


	void mainLoop() {
		// Event Handler
		static auto lastTime = std::chrono::high_resolution_clock::now();
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();

			drawFrame();

			if (swapChainOutdated)
				recreateSwapChain();
			else
				presentFrame();
		}

		vkDeviceWaitIdle(device);
	}

	void cleanup() {
		cleanupSwapChain();

		if (vert != VK_NULL_HANDLE) { vkDestroyShaderModule(device, vert, nullptr); vert = VK_NULL_HANDLE; }
		if (frag != VK_NULL_HANDLE) { vkDestroyShaderModule(device, frag, nullptr); frag = VK_NULL_HANDLE; }

		for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
			// Here there is a choice for the Allocator function
			vkDestroySemaphore(device, renderFinishedSemaphore[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphore[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
		// Here there is a choice for the Allocator function
		vkDestroyCommandPool(device, commandPool, nullptr);
		if (transientcommandPool != commandPool) vkDestroyCommandPool(device, transientcommandPool, nullptr);


		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		vkDestroySurfaceKHR(instance, mainSurface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);	// Cleanup Window Resources

		glfwTerminate();	// Terminate GLFW Library
	}

	void cleanupSwapChain() {
		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
		}
		vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

		for (size_t i = 0; i < mainSwapChain.imageViews.size(); i++) {
			vkDestroyImageView(device, mainSwapChain.imageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(device, mainSwapChain.swapChain, nullptr);

	}

	void recreateSwapChain() {
		swapChainOutdated = false;
		// Swap chain recreation for events like window resizing

		
		// Check the special case where the window is minimized and the framebuffer size is zero
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}


		// First wait for device to be idle so that all resources are free from use
		vkDeviceWaitIdle(device);

		cleanupSwapChain();

		// Recreate the swapchain
		createSwapChain(mainSurface, mainSwapChain);
		createSwapChainImageViews(mainSwapChain);
		// The render pass depends on the format of the swap chain. It is rare that the format changes but to be sure
		createRenderPass(renderPass, mainSwapChain.format, VK_SAMPLE_COUNT_1_BIT, true, true, false, true);
		std::vector<VkPushConstantRange> push(1, { VK_SHADER_STAGE_FRAGMENT_BIT,0,sizeof(PushConstants) });
		createGraphicsPipeline(graphicsPipeline, pipelineLayout, renderPass, vert, frag, constants::NullVertexDescriptions, &push, VK_NULL_HANDLE, mainSwapChain.extent, VK_SAMPLE_COUNT_1_BIT);
		swapChainFramebuffers = createFramebuffers(renderPass, mainSwapChain);
		commandBuffers = createCommandBuffers(commandPool, swapChainFramebuffers.size(),true);
	}

	void drawFrame() {
		// The vkWaitForFences function takes an array of fences and waits for either
		// any or all of them to be signaled before returning.The VK_TRUE we pass here
		// indicates that we want to wait for all fences, but in the case of a single one it
		// obviously doesn’t matter.Just like vkAcquireNextImageKHR this function also
		// takes a timeout.Unlike the semaphores, we manually need to restore the fence
		// to the unsignaled state by resetting it with the vkResetFences call.
		vkWaitForFences(device, 1, &inFlightFences[inFlightFrameIndex], VK_TRUE, UINT64_MAX);

		// The drawFrame function perform three operations:
		//	• Acquire an image from the swap chain
		//	• Execute the command buffer with that image as attachment in the framebuffer
		//	• Return the image to the swap chain for presentation
		// However the functions that acomplish this return before the operation is actually finished and each operation depends
		// on the previous one being finished. To syncronize them we need semaphores which are use for syncronizing operations within
		// or across command queues. If we want to syncronize calls between the application and the rendering operations, we should use fences instead.



		// vkAcquireNextImageKHR arguments
		//• Logical Device
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
		VkResult result = vkAcquireNextImageKHR(device, mainSwapChain.swapChain, UINT64_MAX, imageAvailableSemaphore[inFlightFrameIndex], VK_NULL_HANDLE, &imageIndex);

		// Using the result we can check if the swapchain is out of data and if so we need to recreate it
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			swapChainOutdated = true;
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { // Since we already adquired an image we can still draw even if its suboptimal
			throw std::runtime_error("failed to acquire swap chain image!");
		}


		// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
			vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}
		// Mark the image as now being in use by this frame
		imagesInFlight[imageIndex] = inFlightFences[inFlightFrameIndex];

		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		PushConstants constants = { {(float)width,(float)height}, time };
		writeCommandBuffer(commandBuffers[imageIndex], swapChainFramebuffers[imageIndex], constants);
		std::vector<VkCommandBuffer> submitCommmandBuffers = { commandBuffers[imageIndex] };

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//We want to wait with writing colors to the image until it’s available, so we’re specifying the stage of
		//the graphics pipeline that writes to the color attachment. That means that theoretically the 
		//implementation can already start executing our vertex shader and such while the image is not yet available.
		//Each entry in the waitStages array corresponds to the semaphore with the same index in pWaitSemaphores.
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore[inFlightFrameIndex] };

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		// Commands to execute
		submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommmandBuffers.size());
		submitInfo.pCommandBuffers = submitCommmandBuffers.data();

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore[inFlightFrameIndex] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device, 1, &inFlightFences[inFlightFrameIndex]);

		// Here we submit the command to draw the triangle.
		// We also use the current Frame Fence to signal when the command buffer finishes executing, this way we know when a frame is finished
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[inFlightFrameIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw comand buffer!");
		}

	}

	void presentFrame() {
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		//The first two parameters specify which semaphores to wait on before presentation
		//can happen, just like VkSubmitInfo.
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphore[inFlightFrameIndex];

		VkSwapchainKHR swapChains[] = { mainSwapChain.swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		// vkQueuePresentKHR possible outpus
		//• VK_ERROR_OUT_OF_DATE_KHR : The swap chain has become incompatible with the surface and can no longer be used for rendering.Usually happens
		// after a window resize.
		//• VK_SUBOPTIMAL_KHR : The swap chain can still be used to successfully present to the surface, but the surface properties are no longer matched
		// exactly.
		VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

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

	void writeCommandBuffer(VkCommandBuffer cmd,VkFramebuffer frmBuffer,PushConstants constants) {
		vkResetCommandBuffer(cmd, 0);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = frmBuffer;
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = mainSwapChain.extent;

		std::array<VkClearValue, 1> clearValues = {};
		clearValues[0].color = { 0.0f,0.0f,0.0f,1.0f };

		//Note that the order of clearValues should be identical to the order of your attachments in the subpass. 
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		//upload the matrix to the GPU via push constants
		vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &constants);
		// Draw Vertex
		vkCmdDraw(cmd, 6, 1, 0, 0);

		vkCmdEndRenderPass(cmd);

		if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
};



int main() {

	compileShaders();
	ShaderToyApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;

}