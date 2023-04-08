#include "vulkan_engine.h"
#include "vulkan_vertices.h"
#include "vulkan_descriptors.h"
#include "glfwInteraction.h"
#include "importResources.h"
#include "imgui_impl_glfw.h"
#include <opencv2/opencv.hpp>
#include <chrono>
#include <map>


// To include the functions bodies and avoid linker errors
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map> // To load the models 

#ifdef IMGUI_EXT
const bool enableImgui = true;
#include "vulkan_imgui.h"
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

extern float cameraEye[3];

// Number of parallel descriptor sets to switch between
const int MIRROR_DESCRIPTOR_SET_COUNT = 2;

const std::string MODEL_PATH = "models/cottage_obj.obj";
const std::string TEXTURE_PATH = "textures/texture_1.png";

const std::array<std::string, MAX_SAMPLED_IMAGES> textures = { "textures/texture_1.png", "textures/texture_2.png" };
const std::array<std::string, MAX_SAMPLED_IMAGES> updatedTextures = { "textures/texture_1_1.png", "textures/texture_2_1.png" };

std::vector<uint32_t> indices = {};
std::vector<PCTVertex> vertices = {};


void compileShaders() {
	// TO DO: Add dynamic location of vulkan installation 
	std::string vulkan_glslc_path = STRING(Vulkan_GLSLC_EXECUTABLE);
	system((vulkan_glslc_path + " ./shaders/shader.vert -o ./vert.spv").c_str());

	system((vulkan_glslc_path + " ./shaders/shader.frag -o ./frag.spv").c_str());
}

namespace std {
	template<> struct hash<PCTVertex> {
		size_t operator()(PCTVertex const& vertex) const {
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
	std::unordered_map<PCTVertex, uint32_t> uniqueVertices = {};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			PCTVertex vertex = {};

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
		mipLevels = 3;
		initWindow();
		initVulkan();
		if (enableImgui) VkEImgui_init((VulkanEngine*)this, imGuiBackEnd);
		mainLoop();
		cleanup();
	}
private:
	VkSurfaceKHR mainSurface;
	VkShaderModule vert;
	VkShaderModule frag;
	VkE_Buffer vertexBuffer;
	VkE_Buffer indexBuffer;
	uint32_t indexCount = 0;
	std::vector<VkE_Buffer> uniformBuffers;
	std::array<VkE_Image, MAX_SAMPLED_IMAGES> textureImages;
	std::array<VkE_Image, MAX_SAMPLED_IMAGES> updatedTextureImages;
	uint32_t mipLevels;

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkEImgui_Backend imGuiBackEnd;
	VkEImgui_DeviceObjectsInfo imguiInfo = { false };
	std::vector<VkCommandBuffer> commandBuffers;

	size_t inFlightFrameIndex = 0;
	uint32_t imageIndex;
	bool framebufferResized = false;
	bool swapChainOutdated = false;
	std::array<std::vector<VkDescriptorSet>, MIRROR_DESCRIPTOR_SET_COUNT> descriptorSets;
	int descriptorGroup = 0;

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
		createSurface(window,mainSurface);
		pickPhysicalDevice(mainSurface);
		createLogicalDevice();
		createSwapChain(mainSurface,mainSwapChain);
		createSwapChainImageViews(mainSwapChain);
		if (enableImgui) {
			VkEImgui_setupBackEnd(imGuiBackEnd, (VulkanEngine*)this, mainSwapChain.minImageCount, mainSwapChain.imageCount,MAX_FRAME_IN_FLIGHT);
			VkEImgui_createBackEndObjects((VulkanEngine*) this, imGuiBackEnd,imguiInfo);
		} 
		createRenderPass(renderPass, mainSwapChain.format, maxMSAASamples, true, !enableImgui, true,true);
		createDescriptorSetLayout();
		shaderCode32 vertShaderCode;
		shaderCode32 fragShaderCode;
		char2shaderCode(readFile("./vert.spv"), vertShaderCode);
		char2shaderCode(readFile("./frag.spv"), fragShaderCode);

		vert = createShaderModule(vertShaderCode);
		frag = createShaderModule(fragShaderCode);
		createGraphicsPipeline(graphicsPipeline,pipelineLayout,renderPass, vert, frag, PCTVertex::getDescriptions(), descriptorSetLayout,mainSwapChain.extent,maxMSAASamples);
		createCommandPool(commandPool,graphicsFamily,0);
		// If transfer family and graphics family are the same use the same command pool
		if (graphicsFamily == transferFamily)
			transientcommandPool = commandPool;
		else {
			// Transfer Challenge & Transient Command Pool Challenge:
			// Create a transient pool for short lived command buffers for memory allocation optimizations.
			createCommandPool(transientcommandPool, transferFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		}
		createColorResources();
		createDepthResources();
		swapChainFramebuffers = createFramebuffers(renderPass,mainSwapChain,colorImageView, depthImageView);
		createVertexBuffer(vertices.data(),vertices.size()*sizeof(vertices[0]), vertexBuffer);
		createIndexBuffer(indices.data(),indices.size()*sizeof(indices[0]),indexBuffer);
		indexCount = static_cast<uint32_t>(indices.size());
		commandBuffers = createCommandBuffers(commandPool, swapChainFramebuffers.size());
		createUniformBuffers();
		createTexture(textureImages[0], textures[0]);
		createTexture(updatedTextureImages[0], updatedTextures[0]);
		createTexture(textureImages[1], textures[1]);
		createTexture(updatedTextureImages[1], updatedTextures[1]);

		createDescriptorPool(descriptorPool, 
			mainSwapChain.imageCount * MIRROR_DESCRIPTOR_SET_COUNT, 
			mainSwapChain.imageCount * MAX_SAMPLED_IMAGES * MIRROR_DESCRIPTOR_SET_COUNT,
			mainSwapChain.imageCount * MIRROR_DESCRIPTOR_SET_COUNT);
		createDescriptorSets();
		updateDescriptorSet(textureImages, 0);
		updateDescriptorSet(updatedTextureImages, 1);
		// Write the command buffers after the descriptor sets are updated
		writeCommandBuffers();
		createSyncObjects(syncObjects,mainSwapChain.imageCount);
	}

	void mainLoop() {
		// Event Handler
		static auto lastTime = std::chrono::high_resolution_clock::now();
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();

			if (enableImgui) {
				VkE_Imgui_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				ImGui::ShowDemoWindow();
				ImGui::Begin("Bobby",NULL,0);
				if (ImGui::BeginMenuBar())
				{
					if (ImGui::BeginMenu("File"))
					{
						if (ImGui::MenuItem("Import content")) {
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Edit"))
					{
						if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
						if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
						ImGui::Separator();
						if (ImGui::MenuItem("Cut", "CTRL+X")) {}
						if (ImGui::MenuItem("Copy", "CTRL+C")) {}
						if (ImGui::MenuItem("Paste", "CTRL+V")) {}
						ImGui::EndMenu();
					}
					ImGui::EndMenuBar();
				}
				ImGui::End();
				ImGui::Render();
			}
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

		vkDestroyShaderModule(device, vert, nullptr);
		vkDestroyShaderModule(device, frag, nullptr);

		cleanupSampledImage(textureImages[0]);
		cleanupSampledImage(textureImages[1]);
		cleanupSampledImage(updatedTextureImages[0]);
		cleanupSampledImage(updatedTextureImages[1]);
		// Here there is a choice for the Allocator function
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		// Here there is a choice for the Allocator function
		destroyBufferBundle(vertexBuffer);
		destroyBufferBundle(indexBuffer);

		cleanupSyncObjects(syncObjects);

		// Here there is a choice for the Allocator function
		vkDestroyCommandPool(device, commandPool, nullptr);
		if (transientcommandPool != commandPool) vkDestroyCommandPool(device, transientcommandPool, nullptr);
		
		if (enableImgui) {
			// Resources to destroy when the program ends
			VkEImgui_cleanupBackEndObjects(imGuiBackEnd);
			VkEImgui_Shutdown();
		}

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
			VkEImgui_cleanupSwapChain(imGuiBackEnd);
		}

		for (size_t i = 0; i < mainSwapChain.imageViews.size(); i++) {
			vkDestroyImageView(device, mainSwapChain.imageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(device, mainSwapChain.swapChain, nullptr);

		for (size_t i = 0; i < uniformBuffers.size(); i++) {
			destroyBufferBundle(uniformBuffers[i]);
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
		createSwapChain(mainSurface,mainSwapChain);
		createSwapChainImageViews(mainSwapChain);
		// The render pass depends on the format of the swap chain. It is rare that the format changes but to be sure
		createRenderPass(renderPass, mainSwapChain.format, maxMSAASamples, true, !enableImgui, true, true);
		createDescriptorPool(descriptorPool, 
			mainSwapChain.imageCount * MIRROR_DESCRIPTOR_SET_COUNT, 
			mainSwapChain.imageCount * MAX_SAMPLED_IMAGES * MIRROR_DESCRIPTOR_SET_COUNT,
			mainSwapChain.imageCount * MIRROR_DESCRIPTOR_SET_COUNT);
		createGraphicsPipeline(graphicsPipeline, pipelineLayout,renderPass, vert,frag, PCTVertex::getDescriptions(), descriptorSetLayout, mainSwapChain.extent, maxMSAASamples);
		createColorResources();
		createDepthResources();
		swapChainFramebuffers = createFramebuffers(renderPass,mainSwapChain, colorImageView, depthImageView);
		createUniformBuffers();
		createDescriptorSets();
		updateDescriptorSet(textureImages, 0);
		updateDescriptorSet(updatedTextureImages, 1);
		commandBuffers = createCommandBuffers(commandPool, swapChainFramebuffers.size());
		writeCommandBuffers();
		if (enableImgui) {
			recreateImguiSwapChainObjects(imGuiBackEnd, imguiInfo);
		}
	}

	void createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(mainSwapChain.imageCount);

		for (size_t i = 0; i < mainSwapChain.imageCount; i++) {
			createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
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
		ubo.proj = glm::perspective(glm::radians(60.0f), mainSwapChain.extent.width / (float)mainSwapChain.extent.height, 0.1f, 60.0f);
		ubo.proj[1][1] *= -1;

		void* data;
		vkMapMemory(device, uniformBuffers[currentImage].memory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, uniformBuffers[currentImage].memory);

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

	void createDescriptorSets() {
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

	void updateDescriptorSet(std::array<VkE_Image, MAX_SAMPLED_IMAGES> images, int groupIndex) {


		std::vector<VkDescriptorSet>& descriptorSet = descriptorSets[groupIndex];
		for (size_t i = 0; i < mainSwapChain.imageCount; i++) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBuffers[i].buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			std::array<VkDescriptorImageInfo, MAX_SAMPLED_IMAGES> imagesInfo;
			for (size_t j = 0; j < MAX_SAMPLED_IMAGES; j++) {
				imagesInfo[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imagesInfo[j].imageView = images[j].view;
				imagesInfo[j].sampler = images[j].sampler;
			}

			std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSet[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr; // Optional
			descriptorWrites[0].pTexelBufferView = nullptr; // Optional

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSet[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = static_cast<uint32_t>(imagesInfo.size());
			descriptorWrites[1].pImageInfo = imagesInfo.data();

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
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

		updateUniformBuffer(imageIndex);

		std::vector<VkCommandBuffer> submitCommmandBuffers = { commandBuffers[imageIndex] };
		VkResult err;
		if (enableImgui) {
			err = vkResetCommandBuffer(imGuiBackEnd.commandBuffers[imageIndex], 0);
			check_vk_result(err);
			VkCommandBufferBeginInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			err = vkBeginCommandBuffer(imGuiBackEnd.commandBuffers[imageIndex], &info);
			check_vk_result(err);

			VkClearValue clearValue = { 0.0f,0.0f ,0.0f ,0.0f };
			VkRenderPassBeginInfo imguiRenderPassBeginInfo = {};
			imguiRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			imguiRenderPassBeginInfo.renderPass = imGuiBackEnd.renderPass;
			imguiRenderPassBeginInfo.framebuffer = imGuiBackEnd.frameBuffers[imageIndex];
			imguiRenderPassBeginInfo.renderArea.extent.width = mainSwapChain.extent.width;
			imguiRenderPassBeginInfo.renderArea.extent.height = mainSwapChain.extent.height;
			imguiRenderPassBeginInfo.clearValueCount = 1;
			imguiRenderPassBeginInfo.pClearValues = &clearValue;
			vkCmdBeginRenderPass(imGuiBackEnd.commandBuffers[imageIndex], &imguiRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Record Imgui Draw Data and draw funcs into command buffer
			VkEImgui_RenderDrawData(ImGui::GetDrawData(), imGuiBackEnd.commandBuffers[imageIndex]);
			// Submit command buffer
			vkCmdEndRenderPass(imGuiBackEnd.commandBuffers[imageIndex]);
			err = vkEndCommandBuffer(imGuiBackEnd.commandBuffers[imageIndex]);
			check_vk_result(err);

			submitCommmandBuffers.push_back(imGuiBackEnd.commandBuffers[imageIndex]);
		}

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

	void createTexture(VkE_Image& image, std::string imageFile){
		cv::Mat matImage = loadImage(imageFile);

		createSampledImage(image, matImage.cols, matImage.rows, matImage.elemSize(),(char*) matImage.data, mipLevels, VK_SAMPLE_COUNT_1_BIT);

		matImage.release();
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
			renderPassInfo.renderArea.extent = mainSwapChain.extent;

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

			VkBuffer vertexBuffers[] = { vertexBuffer.buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
			// We can only have a single index buffer.
			vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

			// Descriptor Set bindings.ffs
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &(descriptorSets[descriptorGroup][i]), 0, nullptr);
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