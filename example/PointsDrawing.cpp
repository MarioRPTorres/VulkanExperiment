#include "vulkan_engine.h"
#include "vulkan_vertices.h"
#include "vulkan_descriptors.h"
#include "glfwInteraction.h"
#include "importResources.h"
#include <chrono>
#include <map>

const int vertexCount = 12;

/*
#version 450 core


layout(push_constant) uniform uPushConstant { float time; } pc;

vec2 circleDot(uint glIndex, float time){
	float radius = 0.5;
	// f(x) = -pi + index/indexCount * 2 * pi
	float x = mod(-3.14 + float(glIndex)* 2*3.14/12 , 6.28);
	float t =  x - sin(x-time)/(1+0.5*pow(x-time,2));

	return vec2(radius * cos(t),radius * sin(t));
}



void main()
{
	gl_PointSize = 5.0;
	gl_Position = vec4(circleDot(gl_VertexIndex,pc.time) , 0.0, 1.0);
}
*/
shaderCode32 vert = {
0x07230203, 0x00010000, 0x000d000a, 0x00000058, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
0x0007000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x0000003e, 0x00000046, 0x00030003,
0x00000002, 0x000001c2, 0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79,
0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47, 0x4c474f4f, 0x6e695f45,
0x64756c63, 0x69645f65, 0x74636572, 0x00657669, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000,
0x00070005, 0x0000000e, 0x63726963, 0x6f44656c, 0x31752874, 0x3b31663b, 0x00000000, 0x00040005,
0x0000000c, 0x6e496c67, 0x00786564, 0x00040005, 0x0000000d, 0x656d6974, 0x00000000, 0x00040005,
0x00000010, 0x69646172, 0x00007375, 0x00030005, 0x00000012, 0x00000078, 0x00030005, 0x0000001f,
0x00000074, 0x00060005, 0x0000003c, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006,
0x0000003c, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000003c, 0x00000001,
0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000003c, 0x00000002, 0x435f6c67,
0x4470696c, 0x61747369, 0x0065636e, 0x00070006, 0x0000003c, 0x00000003, 0x435f6c67, 0x446c6c75,
0x61747369, 0x0065636e, 0x00030005, 0x0000003e, 0x00000000, 0x00060005, 0x00000046, 0x565f6c67,
0x65747265, 0x646e4978, 0x00007865, 0x00060005, 0x00000049, 0x73755075, 0x6e6f4368, 0x6e617473,
0x00000074, 0x00050006, 0x00000049, 0x00000000, 0x656d6974, 0x00000000, 0x00030005, 0x0000004b,
0x00006370, 0x00040005, 0x0000004c, 0x61726170, 0x0000006d, 0x00040005, 0x0000004d, 0x61726170,
0x0000006d, 0x00050048, 0x0000003c, 0x00000000, 0x0000000b, 0x00000000, 0x00050048, 0x0000003c,
0x00000001, 0x0000000b, 0x00000001, 0x00050048, 0x0000003c, 0x00000002, 0x0000000b, 0x00000003,
0x00050048, 0x0000003c, 0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000003c, 0x00000002,
0x00040047, 0x00000046, 0x0000000b, 0x0000002a, 0x00050048, 0x00000049, 0x00000000, 0x00000023,
0x00000000, 0x00030047, 0x00000049, 0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
0x00000002, 0x00040015, 0x00000006, 0x00000020, 0x00000000, 0x00040020, 0x00000007, 0x00000007,
0x00000006, 0x00030016, 0x00000008, 0x00000020, 0x00040020, 0x00000009, 0x00000007, 0x00000008,
0x00040017, 0x0000000a, 0x00000008, 0x00000002, 0x00050021, 0x0000000b, 0x0000000a, 0x00000007,
0x00000009, 0x0004002b, 0x00000008, 0x00000011, 0x3f000000, 0x0004002b, 0x00000008, 0x00000013,
0xc048f5c3, 0x0004002b, 0x00000008, 0x00000016, 0x40000000, 0x0004002b, 0x00000008, 0x00000018,
0x4048f5c3, 0x0004002b, 0x00000008, 0x0000001a, 0x41400000, 0x0004002b, 0x00000008, 0x0000001d,
0x40c8f5c3, 0x0004002b, 0x00000008, 0x00000025, 0x3f800000, 0x00040017, 0x00000039, 0x00000008,
0x00000004, 0x0004002b, 0x00000006, 0x0000003a, 0x00000001, 0x0004001c, 0x0000003b, 0x00000008,
0x0000003a, 0x0006001e, 0x0000003c, 0x00000039, 0x00000008, 0x0000003b, 0x0000003b, 0x00040020,
0x0000003d, 0x00000003, 0x0000003c, 0x0004003b, 0x0000003d, 0x0000003e, 0x00000003, 0x00040015,
0x0000003f, 0x00000020, 0x00000001, 0x0004002b, 0x0000003f, 0x00000040, 0x00000001, 0x0004002b,
0x00000008, 0x00000041, 0x40a00000, 0x00040020, 0x00000042, 0x00000003, 0x00000008, 0x0004002b,
0x0000003f, 0x00000044, 0x00000000, 0x00040020, 0x00000045, 0x00000001, 0x0000003f, 0x0004003b,
0x00000045, 0x00000046, 0x00000001, 0x0003001e, 0x00000049, 0x00000008, 0x00040020, 0x0000004a,
0x00000009, 0x00000049, 0x0004003b, 0x0000004a, 0x0000004b, 0x00000009, 0x00040020, 0x0000004e,
0x00000009, 0x00000008, 0x0004002b, 0x00000008, 0x00000052, 0x00000000, 0x00040020, 0x00000056,
0x00000003, 0x00000039, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
0x00000005, 0x0004003b, 0x00000007, 0x0000004c, 0x00000007, 0x0004003b, 0x00000009, 0x0000004d,
0x00000007, 0x00050041, 0x00000042, 0x00000043, 0x0000003e, 0x00000040, 0x0003003e, 0x00000043,
0x00000041, 0x0004003d, 0x0000003f, 0x00000047, 0x00000046, 0x0004007c, 0x00000006, 0x00000048,
0x00000047, 0x0003003e, 0x0000004c, 0x00000048, 0x00050041, 0x0000004e, 0x0000004f, 0x0000004b,
0x00000044, 0x0004003d, 0x00000008, 0x00000050, 0x0000004f, 0x0003003e, 0x0000004d, 0x00000050,
0x00060039, 0x0000000a, 0x00000051, 0x0000000e, 0x0000004c, 0x0000004d, 0x00050051, 0x00000008,
0x00000053, 0x00000051, 0x00000000, 0x00050051, 0x00000008, 0x00000054, 0x00000051, 0x00000001,
0x00070050, 0x00000039, 0x00000055, 0x00000053, 0x00000054, 0x00000052, 0x00000025, 0x00050041,
0x00000056, 0x00000057, 0x0000003e, 0x00000044, 0x0003003e, 0x00000057, 0x00000055, 0x000100fd,
0x00010038, 0x00050036, 0x0000000a, 0x0000000e, 0x00000000, 0x0000000b, 0x00030037, 0x00000007,
0x0000000c, 0x00030037, 0x00000009, 0x0000000d, 0x000200f8, 0x0000000f, 0x0004003b, 0x00000009,
0x00000010, 0x00000007, 0x0004003b, 0x00000009, 0x00000012, 0x00000007, 0x0004003b, 0x00000009,
0x0000001f, 0x00000007, 0x0003003e, 0x00000010, 0x00000011, 0x0004003d, 0x00000006, 0x00000014,
0x0000000c, 0x00040070, 0x00000008, 0x00000015, 0x00000014, 0x00050085, 0x00000008, 0x00000017,
0x00000015, 0x00000016, 0x00050085, 0x00000008, 0x00000019, 0x00000017, 0x00000018, 0x00050088,
0x00000008, 0x0000001b, 0x00000019, 0x0000001a, 0x00050081, 0x00000008, 0x0000001c, 0x00000013,
0x0000001b, 0x0005008d, 0x00000008, 0x0000001e, 0x0000001c, 0x0000001d, 0x0003003e, 0x00000012,
0x0000001e, 0x0004003d, 0x00000008, 0x00000020, 0x00000012, 0x0004003d, 0x00000008, 0x00000021,
0x00000012, 0x0004003d, 0x00000008, 0x00000022, 0x0000000d, 0x00050083, 0x00000008, 0x00000023,
0x00000021, 0x00000022, 0x0006000c, 0x00000008, 0x00000024, 0x00000001, 0x0000000d, 0x00000023,
0x0004003d, 0x00000008, 0x00000026, 0x00000012, 0x0004003d, 0x00000008, 0x00000027, 0x0000000d,
0x00050083, 0x00000008, 0x00000028, 0x00000026, 0x00000027, 0x0007000c, 0x00000008, 0x00000029,
0x00000001, 0x0000001a, 0x00000028, 0x00000016, 0x00050085, 0x00000008, 0x0000002a, 0x00000011,
0x00000029, 0x00050081, 0x00000008, 0x0000002b, 0x00000025, 0x0000002a, 0x00050088, 0x00000008,
0x0000002c, 0x00000024, 0x0000002b, 0x00050083, 0x00000008, 0x0000002d, 0x00000020, 0x0000002c,
0x0003003e, 0x0000001f, 0x0000002d, 0x0004003d, 0x00000008, 0x0000002e, 0x00000010, 0x0004003d,
0x00000008, 0x0000002f, 0x0000001f, 0x0006000c, 0x00000008, 0x00000030, 0x00000001, 0x0000000e,
0x0000002f, 0x00050085, 0x00000008, 0x00000031, 0x0000002e, 0x00000030, 0x0004003d, 0x00000008,
0x00000032, 0x00000010, 0x0004003d, 0x00000008, 0x00000033, 0x0000001f, 0x0006000c, 0x00000008,
0x00000034, 0x00000001, 0x0000000d, 0x00000033, 0x00050085, 0x00000008, 0x00000035, 0x00000032,
0x00000034, 0x00050050, 0x0000000a, 0x00000036, 0x00000031, 0x00000035, 0x000200fe, 0x00000036,
0x00010038
};
/*
#version 450
layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(1.0,1.0,1.0,1.0);
}
*/
shaderCode32 frag = {
0x07230203, 0x00010000, 0x000d000a, 0x0000000c, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
0x0006000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x00030010, 0x00000004,
0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45,
0x74735f70, 0x5f656c79, 0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47,
0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669, 0x00040005, 0x00000004,
0x6e69616d, 0x00000000, 0x00050005, 0x00000009, 0x4374756f, 0x726f6c6f, 0x00000000, 0x00040047,
0x00000009, 0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020,
0x00000008, 0x00000003, 0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x0004002b,
0x00000006, 0x0000000a, 0x3f800000, 0x0007002c, 0x00000007, 0x0000000b, 0x0000000a, 0x0000000a,
0x0000000a, 0x0000000a, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
0x00000005, 0x0003003e, 0x00000009, 0x0000000b, 0x000100fd, 0x00010038
};


class PointsDrawingApplication :public VulkanWindow {
	using VulkanWindow::VulkanWindow;
public:
	void run() {
		mipLevels = 1;
		initWindow("DeepLeap", 500, 500, true, this, nullptr, nullptr);
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
	std::vector<VkE_Buffer> uniformBuffers = {{}};
	std::vector<VkDescriptorSet> descriptorSets;
	float t_value = 0.0f;
	VkPipelineLayout linesPipelineLayout = VK_NULL_HANDLE;
	VkPipeline linesPipeline = VK_NULL_HANDLE;

	uint32_t indexCount = 0;
	VkCommandPool transientCommandPool = VK_NULL_HANDLE;

	void initVulkan() {
		vk->createInstance();
		vk->setupDebugMessenger();
		vk->createSurface(window,surface);
		vk->pickPhysicalDevice(surface);
		vk->createLogicalDevice();
		VulkanBackEndData vkbd = vk->getBackEndData();
		graphicsQueue = vkbd.graphicsQueue;
		presentQueue = vkbd.presentQueue;
		msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		vertShader = vk->createShaderModule(vert);
		fragShader = vk->createShaderModule(frag);

		//createDescriptorSetLayout();
		vk->createCommandPool(commandPool, vkbd.graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		if (vkbd.graphicsQueueFamily == vkbd.transferQueueFamily)
			transientCommandPool = commandPool;
		else {
			vk->createCommandPool(transientCommandPool, vkbd.transferQueueFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		}
		vk->setTransientCommandPool(transientCommandPool);

		//vk->createVertexBuffer(vertices.data(), vertices.size() * sizeof(vertices[0]), vertexBuffer);
		//vk->createIndexBuffer(indices.data(), indices.size() * sizeof(indices[0]), indexBuffer);
		//indexCount = static_cast<uint32_t>(indices.size());

		// Window Rendering objects
		createDescriptorSetLayout();
		createSwapChainObjects();
	}


	void createDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
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

	void createUniformBuffers() {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glm::vec2 windowSize = { width,height };
		VkDeviceSize bufferSize = sizeof(windowSize);
		for (size_t i = 0; i < uniformBuffers.size(); i++) {
			vk->createBufferWithData(&windowSize, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uniformBuffers[i]);
		}
	}

	void createDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(uniformBuffers.size(), descriptorSetLayout);
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

	void updateDescriptorSet() {

		for (size_t i = 0; i < descriptorSets.size(); i++) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBuffers[i].buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(glm::vec2);

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

	void createSwapChainObjects() {
		VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		vk->createSwapChain(surface, sc, extent);
		vk->createSwapChainImageViews(sc);
		vk->createRenderPass(renderPass, sc.format, msaaSamples, true, true, false, true);
		std::vector<VkPushConstantRange> push(1, { VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(float) });
		vk->createGraphicsPipeline(pipeline,
			pipelineLayout,
			renderPass,
			VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
			false,
			vertShader, fragShader,
			constants::NullVertexDescriptions,
			&push,
			descriptorSetLayout,
			sc.extent,
			msaaSamples
		);
		vk->createGraphicsPipeline(linesPipeline,
			linesPipelineLayout,
			renderPass,
			VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
			false,
			vertShader, fragShader,
			constants::NullVertexDescriptions,
			&push,
			VK_NULL_HANDLE,
			sc.extent,
			msaaSamples
		);
		frameBuffers = vk->createFramebuffers(renderPass, sc, nullptr, depthImage.view);
		commandBuffers = vk->createCommandBuffers(commandPool, frameBuffers.size(), true);
		vk->createDescriptorPool(descriptorPool, sc.imageCount, sc.imageCount, sc.imageCount);
		createUniformBuffers();
		createDescriptorSets();
		updateDescriptorSet();
		vk->createSyncObjects(syncObjects, sc.imageCount);
	}


	void cleanupSwapChain() {
		vk->cleanupSyncObjects(syncObjects);


		for (size_t i = 0; i < frameBuffers.size(); i++) {
			vkDestroyFramebuffer(vk->device, frameBuffers[i], nullptr);
		}
		vkFreeCommandBuffers(vk->device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		vkDestroyPipeline(vk->device, pipeline, nullptr);
		vkDestroyPipelineLayout(vk->device, pipelineLayout, nullptr);
		vkDestroyPipeline(vk->device, linesPipeline, nullptr);
		vkDestroyPipelineLayout(vk->device, linesPipelineLayout, nullptr);
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

	void cleanup() {
		cleanupSwapChain();

		if (vertShader != VK_NULL_HANDLE) { vkDestroyShaderModule(vk->device, vertShader, nullptr); vertShader = VK_NULL_HANDLE; }
		if (fragShader != VK_NULL_HANDLE) { vkDestroyShaderModule(vk->device, fragShader, nullptr); fragShader = VK_NULL_HANDLE; }
		// Here there is a choice for the Allocator function
		vkDestroyDescriptorSetLayout(vk->device, descriptorSetLayout, nullptr);
		// Here there is a choice for the Allocator function
		vk->destroyBufferBundle(vertexBuffer);
		vk->destroyBufferBundle(indexBuffer);


		// Here there is a choice for the Allocator function
		vkDestroyCommandPool(vk->device, commandPool, nullptr);
		if (transientCommandPool != commandPool) vkDestroyCommandPool(vk->device, transientCommandPool, nullptr);
		commandPool = VK_NULL_HANDLE;
		transientCommandPool = VK_NULL_HANDLE;

		vk->destroySurface(surface);

		vk->shutdownVulkanEngine();

		glfwDestroyWindow(window);	// Cleanup Window Resources

		glfwTerminate();	// Terminate GLFW Library
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
		
		createSwapChainObjects();
	}

	void drawFrame() {

		vkWaitForFences(vk->device, 1, &syncObjects.inFlightFences[inFlightFrameIndex], VK_TRUE, UINT64_MAX);
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

		writeCommandBuffer(commandBuffers[imageIndex],frameBuffers[imageIndex],t_value);

		std::vector<VkCommandBuffer> submitCommmandBuffers = { commandBuffers[imageIndex] };
		VkResult err;
	

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

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
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, syncObjects.inFlightFences[inFlightFrameIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw comand buffer!");
		}

	}

	void presentFrame() {
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &syncObjects.renderFinishedSemaphore[inFlightFrameIndex];

		VkSwapchainKHR swapChains[] = { sc.swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		
		VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		inFlightFrameIndex = (inFlightFrameIndex + 1) % MAX_FRAME_IN_FLIGHT;
	}

	void writeCommandBuffer(VkCommandBuffer cmd, VkFramebuffer frmBuffer, float time) {
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
		vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float), &time);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[0], 0, nullptr);

		// Draw Vertex
		vkCmdDraw(cmd, vertexCount, 1, 0, 0);

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, linesPipeline);
		//upload the matrix to the GPU via push constants
		vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float), &time);

		// Draw Vertex
		vkCmdDraw(cmd, vertexCount, 1, 0, 0);


		vkCmdEndRenderPass(cmd);

		if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
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

			auto currentTime = std::chrono::high_resolution_clock::now();
			t_value = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
			int r = (int)(t_value / (6.28));
			t_value = t_value - (float)(r)*6.28;
		}

		vkDeviceWaitIdle(vk->device);
	}
};



int main() {
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