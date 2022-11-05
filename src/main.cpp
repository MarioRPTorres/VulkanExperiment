#include "vulkan_engine.h"
#include "glfwInteraction.h"
#include "importResources.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"


#ifdef IMGUI_EXT
const bool enableImgui = false;
#else
const bool enableImgui = false;
#endif

// To include the functions bodies and avoid linker errors
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map> // To load the models 

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
typedef std::vector<char> shaderCode;

const int WIDTH = 640;
const int HEIGHT = 480;

extern glm::vec3 cameraEye;

const std::string MODEL_PATH = "models/cottage_obj.obj";
const std::string TEXTURE_PATH = "textures/texture_1.png";

const std::array<std::string, MAX_SAMPLED_IMAGES> textures = { "textures/texture_1.png", "textures/texture_2.png" };
const std::array<std::string, MAX_SAMPLED_IMAGES> updatedTextures = { "textures/texture_1_1.png", "textures/texture_2_1.png" };

std::vector<uint32_t> indices = {};
std::vector<Vertex> vertices = {};

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

void compileShaders() {
	// TO DO: Add dynamic location of vulkan installation 
	system("C:/.local/VulkanSDK-1.3.204/Bin/glslc ./resources/shader.vert -o ./vert.spv");
	system("C:/.local/VulkanSDK-1.3.204/Bin/glslc ./resources/shader.frag -o ./frag.spv");
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
		if (enableImgui) initImgui();
		mainLoop();
		cleanup();
	}
private:
	shaderCode vert;
	shaderCode frag;

	std::array<SampledImage, MAX_SAMPLED_IMAGES> textureImages;
	std::array<SampledImage, MAX_SAMPLED_IMAGES> updatedTextureImages;
	
	VkRenderPass imguiRenderPass;
	VkCommandPool imguiCommandPool;
	std::vector<VkCommandBuffer> imguiCommandBuffers;

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
		if (enableImgui) createImguiRenderPasses(); else createRenderPass();
		createDescriptorSetLayout();
		vert = readFile("./vert.spv");
		frag = readFile("./frag.spv");
		createGraphicsPipeline(vert, frag);
		createCommandPool();
		if (enableImgui) createImguiCommandPool();
		createColorResources();
		createDepthResources();
		createFramebuffers();
		createVertexBuffer(vertices);
		createIndexBuffer(indices);
		createCommandBuffers();
		createUniformBuffers();
		createSampledImage(textureImages[0], textures[0]);
		createSampledImage(updatedTextureImages[0], updatedTextures[0]);
		createSampledImage(textureImages[1], textures[1]);
		createSampledImage(updatedTextureImages[1], updatedTextures[1]);

		if (enableImgui) createImguiDescriptorPool();	else createDescriptorPool();
		createDescriptorSets();
		updateDescriptorSet(textureImages, 0);
		updateDescriptorSet(updatedTextureImages, 1);
		// Write the command buffers after the descriptor sets are updated
		writeCommandBuffers();
		createSyncObjects();
	}

	void initImgui() {
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = instance;
		init_info.PhysicalDevice = physicalDevice;
		init_info.Device = device;
		init_info.QueueFamily = 0;
		init_info.Queue = graphicsQueue;
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = descriptorPool;
		init_info.Allocator = nullptr;
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
		init_info.MinImageCount = swapChainSupport.capabilities.minImageCount;
		init_info.ImageCount = static_cast<uint32_t>(swapChainImages.size());
		init_info.CheckVkResultFn = check_vk_result;
		ImGui_ImplVulkan_Init(&init_info, imguiRenderPass);

		VkCommandBuffer command_buffer = beginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
		endSingleTimeCommands(command_buffer);
	}

	void mainLoop() {
		// Event Handler
		static auto lastTime = std::chrono::high_resolution_clock::now();
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();


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
		if (enableImgui) vkDestroyCommandPool(device, imguiCommandPool, nullptr);
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
		if (enableImgui) {
			vkDestroyRenderPass(device, imguiRenderPass, nullptr);
			vkFreeCommandBuffers(device, imguiCommandPool, static_cast<uint32_t>(imguiCommandBuffers.size()), imguiCommandBuffers.data());
		}
		vkDestroyRenderPass(device, renderPass, nullptr);

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
		if (enableImgui) {
			createImguiRenderPasses();
			createImguiDescriptorPool();
		}	
		else {
			createRenderPass();
			createDescriptorPool();
		}
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
			createImguiCommandBuffers();
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


		uint32_t imageIndex;

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
			recreateSwapChain();
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
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphore[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device, 1, &inFlightFences[currentFrame]);

		// Here we submit the command to draw the triangle.
		// We also use the current Frame Fence to signal when the command buffer finishes executing, this way we know when a frame is finished
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw comand buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		//The first two parameters specify which semaphores to wait on before presentation
		//can happen, just like VkSubmitInfo.
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

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
		currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
	}

	void createImguiRenderPasses() {
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
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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

		// ****************  Imgui Render Pass **************
		// Color Attachment creation
		VkAttachmentDescription imguiColorAttachment = {};
		imguiColorAttachment.format = swapChainImageFormat;
		imguiColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // msaa is already resolved in previous renderpass and imgui doesn't need it
		imguiColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // We want to keep what is already in the framebuffers and draw over it the imgui widgets
		imguiColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		imguiColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		imguiColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		imguiColorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		imguiColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Reference to point to the attachment in the attachment array
		VkAttachmentReference imguiColorAttachmentRef = {};
		imguiColorAttachmentRef.attachment = 0;
		imguiColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// Create first subpass with one attachment
		VkSubpassDescription imGuiSubpass = {};
		imGuiSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		imGuiSubpass.colorAttachmentCount = 1;
		imGuiSubpass.pColorAttachments = &imguiColorAttachmentRef;

		// Create dependency to synchronize this subpass with the penultimate renderpass
		VkSubpassDependency imguiDependency = {};
		imguiDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		imguiDependency.dstSubpass = 0;
		imguiDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		imguiDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		imguiDependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imguiDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		// Create the Render pass with arguments of the subpasses used and the attachments to refer to.
		VkRenderPassCreateInfo imguiRenderPassInfo = {};
		imguiRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		imguiRenderPassInfo.attachmentCount = 1;
		imguiRenderPassInfo.pAttachments = &imguiColorAttachment;
		imguiRenderPassInfo.subpassCount = 1;
		imguiRenderPassInfo.pSubpasses = &imGuiSubpass;
		imguiRenderPassInfo.dependencyCount = 1;
		imguiRenderPassInfo.pDependencies = &imguiDependency;

		// Here there is a choice for the Allocator function
		if (vkCreateRenderPass(device, &imguiRenderPassInfo, nullptr, &imguiRenderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create imgui render pass!");
		}
	}

	void createImguiDescriptorPool() {
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};


		uint32_t pool_sizes_count = (int)(sizeof(pool_sizes) / sizeof(*pool_sizes));
		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 1000 * pool_sizes_count;
		poolInfo.poolSizeCount = pool_sizes_count;
		poolInfo.pPoolSizes = pool_sizes;

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create imgui descriptor pool!");
		}
	}

	void createImguiCommandPool() {
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allow command buffers to be rerecorded 
		//	individually, without this flag they all have to be reset together

		// Here there is a choice for the Allocator function
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &imguiCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void createImguiCommandBuffers(){
		imguiCommandBuffers.resize(swapChainFramebuffers.size());
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = imguiCommandPool;
		//• VK_COMMAND_BUFFER_LEVEL_PRIMARY : Can be submitted to a queue for execution, but cannot be called from other command buffers.
		//• VK_COMMAND_BUFFER_LEVEL_SECONDARY : Cannot be submitted directly, but can be called from primary command buffers.
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)imguiCommandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, imguiCommandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate imgui command buffers!");
		}
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