#include "vulkan_engine.h"
#include "vulkan_vertices.h"
#include "vulkan_descriptors.h"
#include "glfwInteraction.h"
#include "importResources.h"
#include <chrono>
#include <map>


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

const int WIDTH = 640;
const int HEIGHT = 480;

extern float cameraEye[3];

std::vector<uint32_t> indices = {};
std::vector<PCTVertex> vertices = {};


void compileShaders() {
	// TO DO: Add dynamic location of vulkan installation 
	std::string vulkan_glslc_path = STRING(Vulkan_GLSLC_EXECUTABLE);
	system((vulkan_glslc_path + " ./shaders/shaderPoints.vert -o ./vert.spv").c_str());

	system((vulkan_glslc_path + " ./shaders/shaderPoints.frag -o ./frag.spv").c_str());
}

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

class PointsDrawingApplication :public VulkanWindow {
	using VulkanWindow::VulkanWindow;
public:
	void run() {
		mipLevels = 3;
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}
private:
	VkShaderModule vertShader = VK_NULL_HANDLE;
	VkShaderModule fragShader = VK_NULL_HANDLE;
	VkE_Buffer vertexBuffer;
	VkE_Buffer indexBuffer;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	std::vector<VkE_Buffer> uniformBuffers;
	std::vector<VkDescriptorSet> descriptorSets;

	uint32_t indexCount = 0;

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

		// Set key input callback
		glfwSetKeyCallback(window, key_callback);
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		// Our HelloTriangleApplication object
		auto app = reinterpret_cast<PointsDrawingApplication*>(glfwGetWindowUserPointer(window));
		// This flag is used for explicit resize because it is not garanteed that the driver will send a out-of-date error when the window is resized.
		app->framebufferResized = true;
	}

	void initVulkan() {
		vk->createInstance();
		vk->setupDebugMessenger();
		vk->createSurface(window,surface);
		vk->pickPhysicalDevice(surface);
		msaaSamples = vk->maxMSAASamples;
		vk->createLogicalDevice();
		graphicsQueue = vk->graphicsQueue;
		presentQueue = vk->presentQueue;
		vk->createSwapChain(surface, sc);
		vk->createSwapChainImageViews(sc.images, sc.format, sc.imageViews);
		vk->createRenderPass(renderPass, sc.format, msaaSamples, true, true, true, true);
		createDescriptorSetLayout();
		shaderCode32 vert;
		shaderCode32 frag;
		char2shaderCode(readFile("./vert.spv"), vert);
		char2shaderCode(readFile("./frag.spv"), frag);
		vertShader = vk->createShaderModule(vert);
		fragShader = vk->createShaderModule(frag);
		vk->createGraphicsPipeline(pipeline, pipelineLayout,renderPass, vertShader, fragShader, PCTVertex::getDescriptions(),nullptr,descriptorSetLayout,sc.extent,msaaSamples);
		vk->createCommandPool(commandPool, vk->graphicsFamily, 0);
		// If transfer family and graphics family are the same use the same command pool
		if (vk->graphicsFamily == vk->transferFamily)
			vk->transientcommandPool = commandPool;
		else {
			// Transfer Challenge & Transient Command Pool Challenge:
			// Create a transient pool for short lived command buffers for memory allocation optimizations.
			vk->createCommandPool(vk->transientcommandPool, vk->transferFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		}
		createColorResources();
		createDepthResources();
		frameBuffers = vk->createFramebuffers(renderPass, sc, msaaColorImage.view, depthImage.view);
		vk->createVertexBuffer(vertices.data(), vertices.size() * sizeof(vertices[0]), vertexBuffer);
		vk->createIndexBuffer(indices.data(), indices.size() * sizeof(indices[0]), indexBuffer);
		indexCount = static_cast<uint32_t>(indices.size());
		commandBuffers = vk->createCommandBuffers(commandPool, frameBuffers.size(),true);
		createUniformBuffers();

		vk->createDescriptorPool(descriptorPool, sc.imageCount ,0,sc.imageCount);
		createDescriptorSets();
		updateDescriptorSet();
		// Write the command buffers after the descriptor sets are updated
		writeCommandBuffers();
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

		if (vertShader != VK_NULL_HANDLE) { vkDestroyShaderModule(vk->device, vertShader, nullptr); vertShader = VK_NULL_HANDLE; }
		if (fragShader != VK_NULL_HANDLE) { vkDestroyShaderModule(vk->device, fragShader, nullptr); fragShader = VK_NULL_HANDLE; }
		// Here there is a choice for the Allocator function
		vkDestroyDescriptorSetLayout(vk->device, descriptorSetLayout, nullptr);
		// Here there is a choice for the Allocator function
		vk->destroyBufferBundle(vertexBuffer);
		vk->destroyBufferBundle(indexBuffer);

		vk->cleanupSyncObjects(syncObjects);

		// Here there is a choice for the Allocator function
		vkDestroyCommandPool(vk->device, commandPool, nullptr);
		if (vk->transientcommandPool != commandPool) vkDestroyCommandPool(vk->device, vk->transientcommandPool, nullptr);

		vk->destroySurface(surface);

		vk->shutdownVulkanEngine();

		glfwDestroyWindow(window);	// Cleanup Window Resources

		glfwTerminate();	// Terminate GLFW Library
	}

	void cleanupSwapChain() {
		vk->cleanupSampledImage(msaaColorImage);
		vk->cleanupSampledImage(depthImage);

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

		for (size_t i = 0; i < uniformBuffers.size(); i++) {
			vk->destroyBufferBundle(uniformBuffers[i]);
		}

		vkDestroyDescriptorPool(vk->device, descriptorPool, nullptr);
	}

	void recreateSwapChain() {
		swapChainOutdated = false;
		// Swap chain recreation for events like window resizing

		int width = 0, height = 0;
		// Check the special case where the window is minimized and the framebuffer size is zero
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}


		// First wait for vk->device to be idle so that all resources are free from use
		vkDeviceWaitIdle(vk->device);

		cleanupSwapChain();

		// Recreate the swapchain
		vk->createSwapChain(surface, sc);
		vk->createSwapChainImageViews(sc.images, sc.format, sc.imageViews);
		// The render pass depends on the format of the swap chain. It is rare that the format changes but to be sure
		vk->createRenderPass(renderPass, sc.format, msaaSamples, true, true, true, true);
		vk->createDescriptorPool(descriptorPool, sc.imageCount, 0, sc.imageCount);
		vk->createGraphicsPipeline(pipeline, pipelineLayout,renderPass, vertShader, fragShader, PCTVertex::getDescriptions(),nullptr,descriptorSetLayout,sc.extent,msaaSamples);
		createColorResources();
		createDepthResources();
		frameBuffers = vk->createFramebuffers(renderPass, sc, msaaColorImage.view, depthImage.view);
		createUniformBuffers();
		createDescriptorSets();
		updateDescriptorSet();
		commandBuffers = vk->createCommandBuffers(commandPool, frameBuffers.size(),true);
		writeCommandBuffers();
	}

	void createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(sc.imageCount);

		for (size_t i = 0; i < sc.imageCount; i++) {
			vk->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].buffer,
				uniformBuffers[i].memory);
		}
	}

	void updateUniformBuffer(uint32_t currentImage) {
		auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo = {};
		//ubo.model = glm::translate(glm::rotate(glm::mat4(1.0f), time*glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)),glm::vec3(1.0f,1.0f,0.0f));
		//ubo.model = glm::translate(glm::mat4(0.0f), glm::vec3(1.0f, 1.0f, 0.0f));
		//ubo.model = glm::rotate(glm::mat4(1.0f), time*glm::radians(90.0f), glm::vec3(0.0f, 0.0f,1.0f));
		ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//ubo.model = glm::mat4(1.0f);
		//ubo.model[3][0] = 1.0f;
		//ubo.model[3][1] = 1.0f;
		//ubo.model[3][2] = 0.0f;
		ubo.view = glm::lookAt(glm::vec3(cameraEye[0], cameraEye[1], cameraEye[2]), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		ubo.proj = glm::perspective(glm::radians(60.0f), sc.extent.width / (float)sc.extent.height, 0.1f, 60.0f);
		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(vk->device, uniformBuffers[currentImage].memory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(vk->device, uniformBuffers[currentImage].memory);

	}

	void updateDescriptorSet() {


		for (size_t i = 0; i < sc.imageCount; i++) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBuffers[i].buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			

			std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr; // Optional
			descriptorWrites[0].pTexelBufferView = nullptr; // Optional



			vkUpdateDescriptorSets(vk->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
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

		updateUniformBuffer(imageIndex);

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

	void createDescriptorSetLayout() {
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


		std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(vk->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void createDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(sc.imageCount, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(layouts.size());

		if (vkAllocateDescriptorSets(vk->device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		
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

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 0.0f,0.0f,0.0f,1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

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

			// Descriptor Set bindings.ffs
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &(descriptorSets[i]), 0, nullptr);
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

	generateVertices(vertices, indices);
	// loadModel();
	compileShaders();
	VulkanEngine vk;
	PointsDrawingApplication app(&vk);

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;

}