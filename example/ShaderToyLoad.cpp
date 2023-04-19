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

class ShaderToyApplication :protected VulkanWindow {
	using VulkanWindow::VulkanWindow;
public:
	void run() {
		initWindow("ShaderToy",width, height, true, this, framebufferResizeCallback, nullptr);
		initVulkan();
		mainLoop();
		cleanup();
	}
private:
	int width = WIDTH;
	int height = HEIGHT;
	VkShaderModule vert = VK_NULL_HANDLE;
	VkShaderModule frag = VK_NULL_HANDLE;
	VkCommandPool transientCommandPool = VK_NULL_HANDLE;

	void initVulkan() {
		vk->createInstance();
		vk->setupDebugMessenger();
		vk->createSurface(window,surface);
		vk->pickPhysicalDevice(surface);
		vk->createLogicalDevice();

		auto vkbd = vk->getBackEndData();
		graphicsQueue = vkbd.graphicsQueue;
		presentQueue = vkbd.presentQueue;
		
		std::string vs = "./";
		vs.append(vertexShader);
		vs.append(".spv");
		std::string fs = "./";
		fs.append(fragShader);
		fs.append(".spv");
		shaderCode32 vertCode, fragCode;
		char2shaderCode(readFile(vs), vertCode);
		char2shaderCode(readFile(fs), fragCode);
		vert = vk->createShaderModule(vertCode);
		frag = vk->createShaderModule(fragCode);
		vk->createCommandPool(commandPool, vkbd.graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		// If transfer family and graphics family are the same use the same command pool
		if (vkbd.graphicsQueueFamily == vkbd.transferQueueFamily)
			transientCommandPool = commandPool;
		else {
			// Transfer Challenge & Transient Command Pool Challenge:
			// Create a transient pool for short lived command buffers for memory allocation optimizations.
			vk->createCommandPool(transientCommandPool, vkbd.transferQueueFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		}
		vk->setTransientCommandPool(transientCommandPool);
		createSwapChainObjects();
	}

	void createSwapChainObjects() {

		VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		vk->createSwapChain(surface,sc,extent);
		vk->createSwapChainImageViews(sc);
		vk->createRenderPass(renderPass,sc.format, VK_SAMPLE_COUNT_1_BIT, true, true,false,true);
		std::vector<VkPushConstantRange> push(1, { VK_SHADER_STAGE_FRAGMENT_BIT,0,sizeof(PushConstants) });
		vk->createGraphicsPipeline(pipeline, pipelineLayout, 
			renderPass,
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			false,
			vert, frag, 
			constants::NullVertexDescriptions, 
			&push, VK_NULL_HANDLE, 
			sc.extent, 
			VK_SAMPLE_COUNT_1_BIT);
		frameBuffers = vk->createFramebuffers(renderPass,sc);
		commandBuffers = vk->createCommandBuffers(commandPool, frameBuffers.size(),true);
		vk->createSyncObjects(syncObjects, sc.imageCount);
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

		vkDeviceWaitIdle(vk->device);
	}

	void cleanup() {
		cleanupSwapChain();

		if (vert != VK_NULL_HANDLE) { vkDestroyShaderModule(vk->device, vert, nullptr); vert = VK_NULL_HANDLE; }
		if (frag != VK_NULL_HANDLE) { vkDestroyShaderModule(vk->device, frag, nullptr); frag = VK_NULL_HANDLE; }

		// Here there is a choice for the Allocator function
		vkDestroyCommandPool(vk->device, commandPool, nullptr);
		if (transientCommandPool != commandPool) {
			vkDestroyCommandPool(vk->device, transientCommandPool, nullptr);
			transientCommandPool = VK_NULL_HANDLE;
		}
		commandPool = VK_NULL_HANDLE;

		vk->destroySurface(surface);
		vk->shutdownVulkanEngine();
		glfwDestroyWindow(window);	// Cleanup Window Resources

		glfwTerminate();	// Terminate GLFW Library
	}

	void cleanupSwapChain() {
		vk->cleanupSyncObjects(syncObjects);
		vkFreeCommandBuffers(vk->device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		for (size_t i = 0; i < frameBuffers.size(); i++) {
			vkDestroyFramebuffer(vk->device, frameBuffers[i], nullptr);
		}

		vkDestroyPipeline(vk->device, pipeline, nullptr);
		vkDestroyPipelineLayout(vk->device, pipelineLayout, nullptr);
		vkDestroyRenderPass(vk->device, renderPass, nullptr);

		for (size_t i = 0; i < sc.imageViews.size(); i++) {
			vkDestroyImageView(vk->device, sc.imageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(vk->device, sc.swapChain, nullptr);

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
		vkDeviceWaitIdle(vk->device);

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

		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		PushConstants constants = { {(float)width,(float)height}, time };
		writeCommandBuffer(commandBuffers[imageIndex], frameBuffers[imageIndex], constants);
		std::vector<VkCommandBuffer> submitCommmandBuffers = { commandBuffers[imageIndex] };

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

	}

	void presentFrame() {
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
		renderPassInfo.renderArea.extent = sc.extent;

		std::array<VkClearValue, 1> clearValues = {};
		clearValues[0].color = { 0.0f,0.0f,0.0f,1.0f };

		//Note that the order of clearValues should be identical to the order of your attachments in the subpass. 
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
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
	VulkanEngine vk;
	ShaderToyApplication app(&vk);

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;

}