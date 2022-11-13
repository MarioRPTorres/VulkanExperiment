#include "vulkan_engine.h"
#include "glfwInteraction.h"
#include "importResources.h"
#include "vulkan_imgui.h"
#include <opencv2/opencv.hpp>
#include <chrono>
#include <map>


// To include the functions bodies and avoid linker errors
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map> // To load the models 

#ifdef IMGUI_EXT
const bool enableImgui = true;
#else
const bool enableImgui = false;
#endif

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

extern glm::vec3 cameraEye;

const std::string MODEL_PATH = "models/cottage_obj.obj";
const std::string TEXTURE_PATH = "textures/texture_1.png";

const std::array<std::string, MAX_SAMPLED_IMAGES> textures = { "textures/texture_1.png", "textures/texture_2.png" };
const std::array<std::string, MAX_SAMPLED_IMAGES> updatedTextures = { "textures/texture_1_1.png", "textures/texture_2_1.png" };

std::vector<uint32_t> indices = {};
std::vector<Vertex> vertices = {};

void imguiBuildUI() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();
	ImGui::Render();
}

void compileShaders() {
	// TO DO: Add dynamic location of vulkan installation 
	std::string vulkan_glslc_path = STRING(Vulkan_GLSLC_EXECUTABLE);
	system((vulkan_glslc_path + " ./shaders/shader.vert -o ./vert.spv").c_str());

	system((vulkan_glslc_path + " ./shaders/shader.frag -o ./frag.spv").c_str());
}

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
		return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
			
		}
	};
}

cv::Mat loadImage(std::string imagePath) {
	cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR); // do grayscale processing?

	if (image.data == NULL) {
		throw std::runtime_error("failed to load texture image!");
	}
	cv::cvtColor(image, image, cv::COLOR_BGR2RGBA, 4);

	return image;
}

void loadModel() {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;
	/*
	Faces in OBJ files can actually contain an arbitrary number of vertices, whereas our
	application can only render triangles. Luckily the LoadObj has an optional
	parameter to automatically triangulate such faces, which is enabled by default.
	*/
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
		throw std::runtime_error(warn + err);
	}
	if (!warn.empty()) printf(warn.c_str());
	std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex = {};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};
			
			if (index.texcoord_index < 0) vertex.texCoord = { 0.0f, 0.0f };
			else vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};
			
			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
				printf("%d", index.texcoord_index);
			}
			
			indices.push_back(uniqueVertices[vertex]);
		}
	}
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

class HelloTriangleApplication:protected VulkanEngine {
public:
	void run() {
		initWindow();
		initVulkan();
		if (enableImgui) initImgui((VulkanEngine*)this,imguiObjects);
		mainLoop();
		cleanup();
	}
private:
	shaderCode vert;
	shaderCode frag;

	std::array<SampledImage, MAX_SAMPLED_IMAGES> textureImages;
	std::array<SampledImage, MAX_SAMPLED_IMAGES> updatedTextureImages;
	
	VulkanImgui_DeviceObjects imguiObjects;
	VulkanImgui_DeviceObjectsInfo imguiInfo = { false };

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
		glfwSetFramebufferSizeCallback(window,framebufferResizeCallback);

		// Set key input callback
		glfwSetKeyCallback(window, key_callback);
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		// Our HelloTriangleApplication object
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
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
		if (enableImgui) {
			createImguiDeviceObjects((VulkanEngine*)this, imguiObjects, imguiInfo);
		} 
		createRenderPass();
		createDescriptorSetLayout();
		vert = readFile("./vert.spv");
		frag = readFile("./frag.spv");
		createGraphicsPipeline(vert, frag);
		createCommandPool();
		createColorResources();
		createDepthResources();
		createFramebuffers();
		createVertexBuffer(vertices);
		createIndexBuffer(indices);
		createCommandBuffers();
		createUniformBuffers();
		createTexture(textureImages[0], textures[0]);
		createTexture(updatedTextureImages[0], updatedTextures[0]);
		createTexture(textureImages[1], textures[1]);
		createTexture(updatedTextureImages[1], updatedTextures[1]);

		createDescriptorPool();
		createDescriptorSets();
		updateDescriptorSet(textureImages, 0);
		updateDescriptorSet(updatedTextureImages, 1);
		// Write the command buffers after the descriptor sets are updated
		writeCommandBuffers();
		createSyncObjects();
	}

	void mainLoop() {
		// Event Handler
		static auto lastTime = std::chrono::high_resolution_clock::now();
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();

			if (enableImgui) imguiBuildUI();
			drawFrame();

			// Update and Render additional Platform Windows
			if (enableImgui){
				static ImGuiIO& imguiIo = ImGui::GetIO(); (void)imguiIo;
				if (imguiIo.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
					ImGui::UpdatePlatformWindows();
					ImGui::RenderPlatformWindowsDefault();
				}
			}
			if (swapChainOutdated)
				recreateSwapChain();
			else
				presentFrame();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
			if (time > 5.0f) {
				lastTime = std::chrono::high_resolution_clock::now();
				descriptorGroup = (descriptorGroup + 1) % MIRROR_DESCRIPTOR_SET_COUNT;
				// First wait for device to be idle so that all resources are free from use
				vkDeviceWaitIdle(device);
				writeCommandBuffers();
			}
		}

		vkDeviceWaitIdle(device);
	}

	void cleanup() {
		cleanupSwapChain();

		cleanupSampledImage(textureImages[0]);
		cleanupSampledImage(textureImages[1]);
		cleanupSampledImage(updatedTextureImages[0]);
		cleanupSampledImage(updatedTextureImages[1]);
		// Here there is a choice for the Allocator function
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroyBuffer(device, indexBuffer, nullptr);
		vkFreeMemory(device, indexBufferMemory, nullptr);
		// Here there is a choice for the Allocator function
		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);

		for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
			// Here there is a choice for the Allocator function
			vkDestroySemaphore(device, renderFinishedSemaphore[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphore[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
		// Here there is a choice for the Allocator function
		vkDestroyCommandPool(device, commandPool, nullptr);
		if (transientcommandPool != commandPool) vkDestroyCommandPool(device, transientcommandPool, nullptr);
		
		if (enableImgui) {
			// Resources to destroy when the program ends
			cleanupImguiObjects(device, imguiObjects);
		}

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
		vkDestroyImageView(device, colorImageView, nullptr);
		vkDestroyImage(device, colorImage, nullptr);
		vkFreeMemory(device, colorImageMemory, nullptr);
		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthImageMemory, nullptr);

		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
		}
		vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

		if (enableImgui) {
			cleanupImguiSwapChainObjects(device, imguiObjects);
		}

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(device, swapChain, nullptr);

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
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

		
		// First wait for device to be idle so that all resources are free from use
		vkDeviceWaitIdle(device);

		cleanupSwapChain();

		// Recreate the swapchain
		createSwapChain();
		createSwapChainImageViews();
		// The render pass depends on the format of the swap chain. It is rare that the format changes but to be sure
		createRenderPass();
		createDescriptorPool();
		createGraphicsPipeline(vert,frag);
		createColorResources();
		createDepthResources();
		createFramebuffers();
		createUniformBuffers();
		createDescriptorSets();
		updateDescriptorSet(textureImages, 0);
		updateDescriptorSet(updatedTextureImages, 1);
		createCommandBuffers();
		writeCommandBuffers();
		if (enableImgui) {
			recreateImguiSwapChainObjects((VulkanEngine*)this, imguiObjects, imguiInfo);
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
		ubo.view = glm::lookAt(cameraEye, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		ubo.proj = glm::perspective(glm::radians(60.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 60.0f);
		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, uniformBuffersMemory[currentImage]);

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

		updateUniformBuffer(imageIndex);

		std::vector<VkCommandBuffer> submitCommmandBuffers = { commandBuffers[imageIndex] };
		VkResult err;
		if (enableImgui) {
			err = vkResetCommandBuffer(imguiObjects.commandBuffers[imageIndex], 0);
			check_vk_result(err);
			VkCommandBufferBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			err = vkBeginCommandBuffer(imguiObjects.commandBuffers[imageIndex], &info);
			check_vk_result(err);

			VkClearValue clearValue = { 0.0f,0.0f ,0.0f ,0.0f };
			VkRenderPassBeginInfo imguiRenderPassBeginInfo = {};
			imguiRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			imguiRenderPassBeginInfo.renderPass = imguiObjects.renderPass;
			imguiRenderPassBeginInfo.framebuffer = imguiObjects.frameBuffers[imageIndex];
			imguiRenderPassBeginInfo.renderArea.extent.width = swapChainExtent.width;
			imguiRenderPassBeginInfo.renderArea.extent.height = swapChainExtent.height;
			imguiRenderPassBeginInfo.clearValueCount = 1;
			imguiRenderPassBeginInfo.pClearValues = &clearValue;
			vkCmdBeginRenderPass(imguiObjects.commandBuffers[imageIndex], &imguiRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Record Imgui Draw Data and draw funcs into command buffer
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiObjects.commandBuffers[imageIndex]);
			// Submit command buffer
			vkCmdEndRenderPass(imguiObjects.commandBuffers[imageIndex]);
			err = vkEndCommandBuffer(imguiObjects.commandBuffers[imageIndex]);
			check_vk_result(err);

			submitCommmandBuffers.push_back(imguiObjects.commandBuffers[imageIndex]);
		}

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

	void createRenderPass() {
		// Only difference from the original function is in the final attachment finalLayout. 
		// In order to have an extra render pass after this one for imgui, the final layout of this render pass needs 
		// to be VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		// 
		// 
		// Color Attachment creation
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = msaaSamples;
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
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Without msaa would be VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Reference to point to the attachment in the attachment array
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		// Depth Attachment creation
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = msaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


		// Color Resolve Attachment creation
		VkAttachmentDescription colorAttachmentResolve = {};
		colorAttachmentResolve.format = swapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// If imgui is enabled there is a second render pass after this one that needs this layout
		colorAttachmentResolve.finalLayout = (enableImgui ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR); // Only difference from the original function is here

		// Reference to point to the attachment in the attachment array
		VkAttachmentReference colorAttachmentResolveRef = {};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;



		// Create first subpass with one attachment
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		//The following other types of attachments can be referenced by a subpass :
		//• pInputAttachments : Attachments that are read from a shader
		//• pResolveAttachments : Attachments used for multisampling color attachments
		//• pDepthStencilAttachment : Attachment for depth and stencil data
		//• pPreserveAttachments : Attachments that are not used by this subpass,	but for which the data must be preserved

		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
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

	void createTexture(SampledImage& image, std::string imageFile){
		cv::Mat matImage = loadImage(imageFile);

		createSampledImage(image, matImage.cols, matImage.rows, matImage.elemSize(),(char*) matImage.data);

		matImage.release();
	}
};



int main() {

	generateVertices(vertices,indices);
	// loadModel();
	compileShaders();
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;

}