#include "vulkan_engine.h"
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
	shaderCode vert;
	shaderCode frag;

	uint32_t imageIndex;
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
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createSwapChainImageViews();
		createShaderToyRenderPass();

		std::string vs = "./";
		vs.append(vertexShader);
		vs.append(".spv");
		std::string fs = "./";
		fs.append(fragShader);
		fs.append(".spv");
		char2shaderCode(readFile(vs), vert);
		char2shaderCode(readFile(fs), frag);
		createGraphicsPipeline(vert, frag);
		createShaderToyCommandPool();
		createFramebuffers();
		createCommandBuffers();
		// Write the command buffers after the descriptor sets are updated
		//writeCommandBuffers();
		createSyncObjects();
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
		vkDestroySurfaceKHR(instance, surface, nullptr);
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

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(device, swapChain, nullptr);

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
		createSwapChain();
		createSwapChainImageViews();
		// The render pass depends on the format of the swap chain. It is rare that the format changes but to be sure
		createShaderToyRenderPass();
		createGraphicsPipeline(vert, frag);
		createFramebuffers();
		createCommandBuffers();
		//writeCommandBuffers();
	}

	void createFramebuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			std::array<VkImageView, 1> attachments = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t> (attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			// Here there is a choice for the Allocator function
			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}


	void drawFrame() {
		// The vkWaitForFences function takes an array of fences and waits for either
		// any or all of them to be signaled before returning.The VK_TRUE we pass here
		// indicates that we want to wait for all fences, but in the case of a single one it
		// obviously doesn’t matter.Just like vkAcquireNextImageKHR this function also
		// takes a timeout.Unlike the semaphores, we manually need to restore the fence
		// to the unsignaled state by resetting it with the vkResetFences call.
		vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

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
		VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);

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
		imagesInFlight[imageIndex] = inFlightFences[currentFrame];

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
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore[currentFrame] };

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		// Commands to execute
		submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommmandBuffers.size());
		submitInfo.pCommandBuffers = submitCommmandBuffers.data();

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		// Here we submit the command to draw the triangle.
		// We also use the current Frame Fence to signal when the command buffer finishes executing, this way we know when a frame is finished
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw comand buffer!");
		}

	}

	void presentFrame() {
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		//The first two parameters specify which semaphores to wait on before presentation
		//can happen, just like VkSubmitInfo.
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphore[currentFrame];

		VkSwapchainKHR swapChains[] = { swapChain };
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
		currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
	}

	void createShaderToyRenderPass() {
		// Only difference from the original function is in the final attachment finalLayout. 
		// In order to have an extra render pass after this one for imgui, the final layout of this render pass needs 
		// to be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		// 
		// 
		// Color Attachment creation
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		//• VK_ATTACHMENT_LOAD_OP_DONT_CARE : Existing contents are undefined;
		//• VK_ATTACHMENT_LOAD_OP_LOAD : Preserve the existing contents of the attachment
		//• VK_ATTACHMENT_LOAD_OP_CLEAR : Clear the values to a constant at the	start
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
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
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Without msaa would be VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Reference to point to the attachment in the attachment array
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		// Create first subpass with one attachment
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = nullptr;
		subpass.pResolveAttachments = nullptr;

		//The following other types of attachments can be referenced by a subpass :
		//• pInputAttachments : Attachments that are read from a shader
		//• pResolveAttachments : Attachments used for multisampling color attachments
		//• pDepthStencilAttachment : Attachment for depth and stencil data
		//• pPreserveAttachments : Attachments that are not used by this subpass,	but for which the data must be preserved

		std::array<VkAttachmentDescription, 1> attachments = { colorAttachment };
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

		// Here there is a choice for the Allocator function
		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}
	void createGraphicsPipeline(shaderCode vert, shaderCode frag) {

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
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;


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
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
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

		//setup push constants
		VkPushConstantRange push_constant;
		//this push constant range starts at the beginning
		push_constant.offset = 0;
		//this push constant range takes up the size of a MeshPushConstants struct
		push_constant.size = sizeof(PushConstants);
		//this push constant range is accessible only in the vertex shader
		push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Uniform values in shaders are similar to dynamics states. Can be changed at drawing time to alter shaders
		// without recreating them. Most commonly used for transformation matrix and texture samplers.
		// Can be specified at the pipeline creation with a VkPipelineLayout object. This is required with or without using uniform values.
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1; 
		pipelineLayoutInfo.pPushConstantRanges = &push_constant;

		// Here there is a choice for the Allocator function
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}


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
		pipelineInfo.pDepthStencilState = nullptr; // Optional
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

	void createShaderToyCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value;
		// Command Pool Flags
		//• VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new 
		//	commands very often(may change memory allocation behavior)
		//• VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : Allow command buffers to be rerecorded 
		//	individually, without this flag they all have to be reset together
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

		// Here there is a choice for the Allocator function
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}

		if (queueFamilyIndices.sharedTransfer())
			transientcommandPool = commandPool;
		else {
			// Transfer Challenge & Transient Command Pool Challenge:
			// Create a transient pool for short lived command buffers for memory allocation optimizations.
			VkCommandPoolCreateInfo transientpoolInfo = {};
			transientpoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			transientpoolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value;
			transientpoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

			// Here there is a choice for the Allocator function
			if (vkCreateCommandPool(device, &transientpoolInfo, nullptr, &transientcommandPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create transient command pool!");
			}
		}
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
		renderPassInfo.renderArea.extent = swapChainExtent;

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

	void writeCommandBuffers() {

		vkResetCommandPool(device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
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
			renderPassInfo.framebuffer = swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = swapChainExtent;

			std::array<VkClearValue, 1> clearValues = {};
			clearValues[0].color = { 0.0f,0.0f,0.0f,1.0f };

			//Note that the order of clearValues should be identical to the order of your attachments in the subpass. 
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			//All of the functions that record commands can be recognized by their vkCmd prefix.

			//• VK_SUBPASS_CONTENTS_INLINE : The render pass commands will be embedded in the primary command 
			//	buffer itself and no secondary command buffers will be executed.
			//• VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : The render pass commands will be executed
			//	from secondary command buffers.
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

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


			// Descriptor Set bindings.ffs
			//vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &(descriptorSets[descriptorGroup][i]), 0, nullptr);
			// Draw Vertex
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
			// Indexed Vertex Draw
			//vkCmdDrawIndexed(commandBuffers[i], indexCount, 1, 0, 0, 0);


			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
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