#include "vulkan_engine.h"
#include "vulkan_descriptors.h"
#include "vulkan_vertices.h"
#include <opencv2/opencv.hpp>
#include <chrono>

/*
#version 450 core

vec2 positions[6] = vec2[](
vec2(-0.25, -0.25),
vec2(-0.25, 0.25),
vec2(0.25, -0.25),
vec2(-0.25, 0.25),
vec2(0.25, 0.25),
vec2(0.25, -0.25)
);
layout(binding = 1) uniform UniforBufferObject {
	vec2 iResolution;
} ubo;
layout(push_constant) uniform uPushConstant { float theta; } pc;

vec2 rotate(vec2 uv, float rotation) {
	float sine = sin(rotation);
	float cosine = cos(rotation);
	//float sine3 = sin(3*rotation);
	//float cosine3 = cos(3*rotation);

	//return vec2((uv.x * cosine3) - (uv.y * sine3) - cosine/2,((uv.x * sine3 )+ (uv.y * cosine3))*ubo.iResolution.x/ubo.iResolution.y - sine/2);
	return vec2((uv.x * cosine) + (uv.y * sine) - cosine/2,(-(uv.x * sine )+ (uv.y * cosine))*ubo.iResolution.x/ubo.iResolution.y - sine/2);
}

void main()
{
	gl_Position = vec4(rotate(positions[gl_VertexIndex],pc.theta) , 0.0, 1.0);
}
*/
shaderCode32 vert = {
0x07230203, 0x00010000, 0x000d000a, 0x00000067, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
0x0007000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x00000050, 0x00000052, 0x00030003,
0x00000002, 0x000001c2, 0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79,
0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47, 0x4c474f4f, 0x6e695f45,
0x64756c63, 0x69645f65, 0x74636572, 0x00657669, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000,
0x00060005, 0x0000000d, 0x61746f72, 0x76286574, 0x663b3266, 0x00003b31, 0x00030005, 0x0000000b,
0x00007675, 0x00050005, 0x0000000c, 0x61746f72, 0x6e6f6974, 0x00000000, 0x00050005, 0x00000013,
0x69736f70, 0x6e6f6974, 0x00000073, 0x00040005, 0x0000001b, 0x656e6973, 0x00000000, 0x00040005,
0x0000001e, 0x69736f63, 0x0000656e, 0x00070005, 0x0000003a, 0x66696e55, 0x7542726f, 0x72656666,
0x656a624f, 0x00007463, 0x00060006, 0x0000003a, 0x00000000, 0x73655269, 0x74756c6f, 0x006e6f69,
0x00030005, 0x0000003c, 0x006f6275, 0x00060005, 0x0000004e, 0x505f6c67, 0x65567265, 0x78657472,
0x00000000, 0x00060006, 0x0000004e, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006,
0x0000004e, 0x00000001, 0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000004e,
0x00000002, 0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e, 0x00070006, 0x0000004e, 0x00000003,
0x435f6c67, 0x446c6c75, 0x61747369, 0x0065636e, 0x00030005, 0x00000050, 0x00000000, 0x00060005,
0x00000052, 0x565f6c67, 0x65747265, 0x646e4978, 0x00007865, 0x00060005, 0x00000054, 0x73755075,
0x6e6f4368, 0x6e617473, 0x00000074, 0x00050006, 0x00000054, 0x00000000, 0x74656874, 0x00000061,
0x00030005, 0x00000056, 0x00006370, 0x00040005, 0x00000057, 0x61726170, 0x0000006d, 0x00040005,
0x0000005b, 0x61726170, 0x0000006d, 0x00050048, 0x0000003a, 0x00000000, 0x00000023, 0x00000000,
0x00030047, 0x0000003a, 0x00000002, 0x00040047, 0x0000003c, 0x00000022, 0x00000000, 0x00040047,
0x0000003c, 0x00000021, 0x00000001, 0x00050048, 0x0000004e, 0x00000000, 0x0000000b, 0x00000000,
0x00050048, 0x0000004e, 0x00000001, 0x0000000b, 0x00000001, 0x00050048, 0x0000004e, 0x00000002,
0x0000000b, 0x00000003, 0x00050048, 0x0000004e, 0x00000003, 0x0000000b, 0x00000004, 0x00030047,
0x0000004e, 0x00000002, 0x00040047, 0x00000052, 0x0000000b, 0x0000002a, 0x00050048, 0x00000054,
0x00000000, 0x00000023, 0x00000000, 0x00030047, 0x00000054, 0x00000002, 0x00020013, 0x00000002,
0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007,
0x00000006, 0x00000002, 0x00040020, 0x00000008, 0x00000007, 0x00000007, 0x00040020, 0x00000009,
0x00000007, 0x00000006, 0x00050021, 0x0000000a, 0x00000007, 0x00000008, 0x00000009, 0x00040015,
0x0000000f, 0x00000020, 0x00000000, 0x0004002b, 0x0000000f, 0x00000010, 0x00000006, 0x0004001c,
0x00000011, 0x00000007, 0x00000010, 0x00040020, 0x00000012, 0x00000006, 0x00000011, 0x0004003b,
0x00000012, 0x00000013, 0x00000006, 0x0004002b, 0x00000006, 0x00000014, 0xbe800000, 0x0005002c,
0x00000007, 0x00000015, 0x00000014, 0x00000014, 0x0004002b, 0x00000006, 0x00000016, 0x3e800000,
0x0005002c, 0x00000007, 0x00000017, 0x00000014, 0x00000016, 0x0005002c, 0x00000007, 0x00000018,
0x00000016, 0x00000014, 0x0005002c, 0x00000007, 0x00000019, 0x00000016, 0x00000016, 0x0009002c,
0x00000011, 0x0000001a, 0x00000015, 0x00000017, 0x00000018, 0x00000017, 0x00000019, 0x00000018,
0x0004002b, 0x0000000f, 0x00000021, 0x00000000, 0x0004002b, 0x0000000f, 0x00000026, 0x00000001,
0x0004002b, 0x00000006, 0x0000002d, 0x40000000, 0x0003001e, 0x0000003a, 0x00000007, 0x00040020,
0x0000003b, 0x00000002, 0x0000003a, 0x0004003b, 0x0000003b, 0x0000003c, 0x00000002, 0x00040015,
0x0000003d, 0x00000020, 0x00000001, 0x0004002b, 0x0000003d, 0x0000003e, 0x00000000, 0x00040020,
0x0000003f, 0x00000002, 0x00000006, 0x00040017, 0x0000004c, 0x00000006, 0x00000004, 0x0004001c,
0x0000004d, 0x00000006, 0x00000026, 0x0006001e, 0x0000004e, 0x0000004c, 0x00000006, 0x0000004d,
0x0000004d, 0x00040020, 0x0000004f, 0x00000003, 0x0000004e, 0x0004003b, 0x0000004f, 0x00000050,
0x00000003, 0x00040020, 0x00000051, 0x00000001, 0x0000003d, 0x0004003b, 0x00000051, 0x00000052,
0x00000001, 0x0003001e, 0x00000054, 0x00000006, 0x00040020, 0x00000055, 0x00000009, 0x00000054,
0x0004003b, 0x00000055, 0x00000056, 0x00000009, 0x00040020, 0x00000058, 0x00000006, 0x00000007,
0x00040020, 0x0000005c, 0x00000009, 0x00000006, 0x0004002b, 0x00000006, 0x00000060, 0x00000000,
0x0004002b, 0x00000006, 0x00000061, 0x3f800000, 0x00040020, 0x00000065, 0x00000003, 0x0000004c,
0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003b,
0x00000008, 0x00000057, 0x00000007, 0x0004003b, 0x00000009, 0x0000005b, 0x00000007, 0x0003003e,
0x00000013, 0x0000001a, 0x0004003d, 0x0000003d, 0x00000053, 0x00000052, 0x00050041, 0x00000058,
0x00000059, 0x00000013, 0x00000053, 0x0004003d, 0x00000007, 0x0000005a, 0x00000059, 0x0003003e,
0x00000057, 0x0000005a, 0x00050041, 0x0000005c, 0x0000005d, 0x00000056, 0x0000003e, 0x0004003d,
0x00000006, 0x0000005e, 0x0000005d, 0x0003003e, 0x0000005b, 0x0000005e, 0x00060039, 0x00000007,
0x0000005f, 0x0000000d, 0x00000057, 0x0000005b, 0x00050051, 0x00000006, 0x00000062, 0x0000005f,
0x00000000, 0x00050051, 0x00000006, 0x00000063, 0x0000005f, 0x00000001, 0x00070050, 0x0000004c,
0x00000064, 0x00000062, 0x00000063, 0x00000060, 0x00000061, 0x00050041, 0x00000065, 0x00000066,
0x00000050, 0x0000003e, 0x0003003e, 0x00000066, 0x00000064, 0x000100fd, 0x00010038, 0x00050036,
0x00000007, 0x0000000d, 0x00000000, 0x0000000a, 0x00030037, 0x00000008, 0x0000000b, 0x00030037,
0x00000009, 0x0000000c, 0x000200f8, 0x0000000e, 0x0004003b, 0x00000009, 0x0000001b, 0x00000007,
0x0004003b, 0x00000009, 0x0000001e, 0x00000007, 0x0004003d, 0x00000006, 0x0000001c, 0x0000000c,
0x0006000c, 0x00000006, 0x0000001d, 0x00000001, 0x0000000d, 0x0000001c, 0x0003003e, 0x0000001b,
0x0000001d, 0x0004003d, 0x00000006, 0x0000001f, 0x0000000c, 0x0006000c, 0x00000006, 0x00000020,
0x00000001, 0x0000000e, 0x0000001f, 0x0003003e, 0x0000001e, 0x00000020, 0x00050041, 0x00000009,
0x00000022, 0x0000000b, 0x00000021, 0x0004003d, 0x00000006, 0x00000023, 0x00000022, 0x0004003d,
0x00000006, 0x00000024, 0x0000001e, 0x00050085, 0x00000006, 0x00000025, 0x00000023, 0x00000024,
0x00050041, 0x00000009, 0x00000027, 0x0000000b, 0x00000026, 0x0004003d, 0x00000006, 0x00000028,
0x00000027, 0x0004003d, 0x00000006, 0x00000029, 0x0000001b, 0x00050085, 0x00000006, 0x0000002a,
0x00000028, 0x00000029, 0x00050081, 0x00000006, 0x0000002b, 0x00000025, 0x0000002a, 0x0004003d,
0x00000006, 0x0000002c, 0x0000001e, 0x00050088, 0x00000006, 0x0000002e, 0x0000002c, 0x0000002d,
0x00050083, 0x00000006, 0x0000002f, 0x0000002b, 0x0000002e, 0x00050041, 0x00000009, 0x00000030,
0x0000000b, 0x00000021, 0x0004003d, 0x00000006, 0x00000031, 0x00000030, 0x0004003d, 0x00000006,
0x00000032, 0x0000001b, 0x00050085, 0x00000006, 0x00000033, 0x00000031, 0x00000032, 0x0004007f,
0x00000006, 0x00000034, 0x00000033, 0x00050041, 0x00000009, 0x00000035, 0x0000000b, 0x00000026,
0x0004003d, 0x00000006, 0x00000036, 0x00000035, 0x0004003d, 0x00000006, 0x00000037, 0x0000001e,
0x00050085, 0x00000006, 0x00000038, 0x00000036, 0x00000037, 0x00050081, 0x00000006, 0x00000039,
0x00000034, 0x00000038, 0x00060041, 0x0000003f, 0x00000040, 0x0000003c, 0x0000003e, 0x00000021,
0x0004003d, 0x00000006, 0x00000041, 0x00000040, 0x00050085, 0x00000006, 0x00000042, 0x00000039,
0x00000041, 0x00060041, 0x0000003f, 0x00000043, 0x0000003c, 0x0000003e, 0x00000026, 0x0004003d,
0x00000006, 0x00000044, 0x00000043, 0x00050088, 0x00000006, 0x00000045, 0x00000042, 0x00000044,
0x0004003d, 0x00000006, 0x00000046, 0x0000001b, 0x00050088, 0x00000006, 0x00000047, 0x00000046,
0x0000002d, 0x00050083, 0x00000006, 0x00000048, 0x00000045, 0x00000047, 0x00050050, 0x00000007,
0x00000049, 0x0000002f, 0x00000048, 0x000200fe, 0x00000049, 0x00010038
};
/*
#version 450
layout(binding = 0) uniform sampler2D texSampler;
layout(binding = 1) uniform UniforBufferObject {
	vec2 iResolution;
} ubo;

layout(location = 0) out vec4 outColor;

void main() {

	vec2 uv = (gl_FragCoord-0.5).xy/ubo.iResolution.xy;
	outColor = texture(texSampler,uv);
}
*/
shaderCode32 frag = {
0x07230203, 0x00010000, 0x000d000a, 0x00000024, 0x00000000, 0x00020011, 0x00000001, 0x0006000b,
0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001,
0x0007000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x0000000c, 0x0000001c, 0x00030010,
0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x000a0004, 0x475f4c47, 0x4c474f4f,
0x70635f45, 0x74735f70, 0x5f656c79, 0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004,
0x475f4c47, 0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669, 0x00040005,
0x00000004, 0x6e69616d, 0x00000000, 0x00030005, 0x00000009, 0x00007675, 0x00060005, 0x0000000c,
0x465f6c67, 0x43676172, 0x64726f6f, 0x00000000, 0x00070005, 0x00000012, 0x66696e55, 0x7542726f,
0x72656666, 0x656a624f, 0x00007463, 0x00060006, 0x00000012, 0x00000000, 0x73655269, 0x74756c6f,
0x006e6f69, 0x00030005, 0x00000014, 0x006f6275, 0x00050005, 0x0000001c, 0x4374756f, 0x726f6c6f,
0x00000000, 0x00050005, 0x00000020, 0x53786574, 0x6c706d61, 0x00007265, 0x00040047, 0x0000000c,
0x0000000b, 0x0000000f, 0x00050048, 0x00000012, 0x00000000, 0x00000023, 0x00000000, 0x00030047,
0x00000012, 0x00000002, 0x00040047, 0x00000014, 0x00000022, 0x00000000, 0x00040047, 0x00000014,
0x00000021, 0x00000001, 0x00040047, 0x0000001c, 0x0000001e, 0x00000000, 0x00040047, 0x00000020,
0x00000022, 0x00000000, 0x00040047, 0x00000020, 0x00000021, 0x00000000, 0x00020013, 0x00000002,
0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007,
0x00000006, 0x00000002, 0x00040020, 0x00000008, 0x00000007, 0x00000007, 0x00040017, 0x0000000a,
0x00000006, 0x00000004, 0x00040020, 0x0000000b, 0x00000001, 0x0000000a, 0x0004003b, 0x0000000b,
0x0000000c, 0x00000001, 0x0004002b, 0x00000006, 0x0000000e, 0x3f000000, 0x0003001e, 0x00000012,
0x00000007, 0x00040020, 0x00000013, 0x00000002, 0x00000012, 0x0004003b, 0x00000013, 0x00000014,
0x00000002, 0x00040015, 0x00000015, 0x00000020, 0x00000001, 0x0004002b, 0x00000015, 0x00000016,
0x00000000, 0x00040020, 0x00000017, 0x00000002, 0x00000007, 0x00040020, 0x0000001b, 0x00000003,
0x0000000a, 0x0004003b, 0x0000001b, 0x0000001c, 0x00000003, 0x00090019, 0x0000001d, 0x00000006,
0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x0003001b, 0x0000001e,
0x0000001d, 0x00040020, 0x0000001f, 0x00000000, 0x0000001e, 0x0004003b, 0x0000001f, 0x00000020,
0x00000000, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005,
0x0004003b, 0x00000008, 0x00000009, 0x00000007, 0x0004003d, 0x0000000a, 0x0000000d, 0x0000000c,
0x00070050, 0x0000000a, 0x0000000f, 0x0000000e, 0x0000000e, 0x0000000e, 0x0000000e, 0x00050083,
0x0000000a, 0x00000010, 0x0000000d, 0x0000000f, 0x0007004f, 0x00000007, 0x00000011, 0x00000010,
0x00000010, 0x00000000, 0x00000001, 0x00050041, 0x00000017, 0x00000018, 0x00000014, 0x00000016,
0x0004003d, 0x00000007, 0x00000019, 0x00000018, 0x00050088, 0x00000007, 0x0000001a, 0x00000011,
0x00000019, 0x0003003e, 0x00000009, 0x0000001a, 0x0004003d, 0x0000001e, 0x00000021, 0x00000020,
0x0004003d, 0x00000007, 0x00000022, 0x00000009, 0x00050057, 0x0000000a, 0x00000023, 0x00000021,
0x00000022, 0x0003003e, 0x0000001c, 0x00000023, 0x000100fd, 0x00010038
};


cv::Mat loadImage(std::string imagePath) {
	cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR); // do grayscale processing?

	if (image.data == NULL) {
		throw std::runtime_error("failed to load texture image!");
	}
	cv::cvtColor(image, image, cv::COLOR_BGR2RGBA, 4);

	return image;
}

class PeekaBooApp : protected VulkanWindow {
	using VulkanWindow::VulkanWindow;
private:
	VkShaderModule vertexShader = VK_NULL_HANDLE;
	VkShaderModule fragmentShader = VK_NULL_HANDLE;
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	//VkCommandPool transientPool = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> descriptorSets = { VK_NULL_HANDLE };
	std::vector<VkE_Buffer> uniformBuffers = { {} };
	VkCommandPool secondaryCommandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> primaryCommandBuffers = {};
	std::vector<VkCommandBuffer> prePushCommandBuffers = {};
	std::vector<VkCommandBuffer> pushCommandBuffers = {};
	std::vector<VkCommandBuffer> afterPushCommandBuffers = {};
	VkE_Image texture;
	float angle = 0.0f;

	void init() {
		msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		initWindow("PeekaBoo", width, height, true, this, framebufferResizeCallback, nullptr);
		
		// Vulkan Setup
		vk->createInstance();
		vk->setupDebugMessenger();
		vk->createSurface(window, surface);
		vk->pickPhysicalDevice(surface);
		vk->createLogicalDevice();
		auto vkbd = vk->getBackEndData();
		graphicsQueue = vkbd.graphicsQueue;
		presentQueue = vkbd.presentQueue;
		vk->createCommandPool(commandPool, vkbd.graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		vk->createCommandPool(secondaryCommandPool, vkbd.transferQueueFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		vk->setTransientCommandPool( secondaryCommandPool);
	
		// Window Rendering objects
		createDescriptorSetLayout();
		vertexShader = vk->createShaderModule(vert);
		fragmentShader = vk->createShaderModule(frag);
		createTexture(texture, "textures/texture_1_1.png");
		createSwapChainObjects();
	}

	void createTexture(VkE_Image& image, std::string imageFile) {
		cv::Mat matImage = loadImage(imageFile);

		vk->createSampledImage(image, matImage.cols, matImage.rows, matImage.elemSize(), (char*)matImage.data, mipLevels, VK_SAMPLE_COUNT_1_BIT);

		matImage.release();
	}

	void createDescriptorSetLayout() {

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Create a descriptor which is used by the shader to access resources like images and buffers
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

		std::array<VkDescriptorSetLayoutBinding, 2> bindings { samplerLayoutBinding,uboLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(vk->device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void updateUniformBuffer(VkE_Buffer buffer) {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glm::vec2 windowSize = { width,height };
		VkDeviceSize bufferSize = sizeof(windowSize);

		void* data;
		vkMapMemory(vk->device, buffer.memory, 0, bufferSize, 0, &data);
		memcpy(data, &windowSize, bufferSize);
		vkUnmapMemory(vk->device, buffer.memory);
	}

	void createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(glm::vec2);

		for (size_t i = 0; i < uniformBuffers.size(); i++) {
			vk->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].buffer,
				uniformBuffers[i].memory);
			updateUniformBuffer(uniformBuffers[i]);
		}

	}

	void createDescriptorSets() {
		std::vector<VkDescriptorSetLayout> layouts(1, descriptorSetLayout);
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

	void writeDescriptorSets() {
		for (size_t i = 0; i < descriptorSets.size(); i++) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBuffers[i].buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(glm::vec2);

			VkDescriptorImageInfo imagesInfo = {};
			imagesInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imagesInfo.imageView = texture.view;
			imagesInfo.sampler = texture.sampler;


			std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
			
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 0;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imagesInfo;
	
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 1;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr; // Optional
			descriptorWrites[0].pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(vk->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void writeUniformBufferDescriptorSet() {
		for (size_t i = 0; i < descriptorSets.size(); i++) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBuffers[i].buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(glm::vec2);

			VkWriteDescriptorSet descriptorWrites = {};

			descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites.dstSet = descriptorSets[i];
			descriptorWrites.dstBinding = 1;
			descriptorWrites.dstArrayElement = 0;
			descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites.descriptorCount = 1;
			descriptorWrites.pBufferInfo = &bufferInfo;
			descriptorWrites.pImageInfo = nullptr; // Optional
			descriptorWrites.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(vk->device, 1, &descriptorWrites, 0, nullptr);
		}
	}

	//void writeSecondaryCommandBuffer(uint32_t currentImage) {
	//	static float theta = 0.0f;
	//	theta += 0.1f;

	//	VkCommandBufferInheritanceInfo inheritanceInfo;
	//	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	//	inheritanceInfo.renderPass = renderPass;
	//	inheritanceInfo.framebuffer = frameBuffers[currentImage];
	//	inheritanceInfo.subpass = 0;
	//	inheritanceInfo.pNext = nullptr;
	//	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	//	inheritanceInfo.queryFlags = 0;
	//	inheritanceInfo.pipelineStatistics = 0;
	//	
	//	vkResetCommandBuffer(pushCommandBuffers[currentImage], 0);
	//	VkCommandBufferBeginInfo beginInfo = {};
	//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	//	beginInfo.pInheritanceInfo = &inheritanceInfo; // Optional
	//	if (vkBeginCommandBuffer(pushCommandBuffers[currentImage], &beginInfo) != VK_SUCCESS) {
	//		throw std::runtime_error("failed to begin recording secondary command buffer!");
	//	}
	//	vkCmdBindPipeline(pushCommandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	//	vkCmdPushConstants(pushCommandBuffers[currentImage], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float), &theta);
	//	if (vkEndCommandBuffer(pushCommandBuffers[currentImage]) != VK_SUCCESS) {
	//		throw std::runtime_error("failed to record secondary command buffer!");
	//	}
	//}

	void writeCommandBuffer(uint32_t index,float theta) {
		vkResetCommandBuffer(primaryCommandBuffers[index],0);

		//vkResetCommandPool(vk->device, commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
			//writeSecondaryCommandBuffer(i);

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

		if (vkBeginCommandBuffer(primaryCommandBuffers[index], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = frameBuffers[index];
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
		vkCmdBeginRenderPass(primaryCommandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


		//float theta = 1.0f;
		//vkCmdPushConstants(primaryCommandBuffers[i], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float), &theta);
		{
			//VkCommandBufferInheritanceInfo inheritanceInfo;
			//inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			//inheritanceInfo.renderPass = renderPass;
			//inheritanceInfo.framebuffer = frameBuffers[i];
			//inheritanceInfo.subpass = 0;
			//inheritanceInfo.pNext = nullptr;
			//inheritanceInfo.occlusionQueryEnable = VK_FALSE;
			//inheritanceInfo.queryFlags = 0;
			//inheritanceInfo.pipelineStatistics = 0;

			//VkCommandBufferBeginInfo beginInfo = {};
			//beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
			//beginInfo.pInheritanceInfo = &inheritanceInfo; // Optional

			//// Pre Push
			//if (vkBeginCommandBuffer(prePushCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			//	throw std::runtime_error("failed to begin recording pre push secondary command buffer!");
			//}
			//vkCmdBindPipeline(prePushCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			//if (vkEndCommandBuffer(prePushCommandBuffers[i]) != VK_SUCCESS) {
			//	throw std::runtime_error("failed to record pre push secondary command buffer!");
			//}
		}

		// After Push 
		//{
		//	VkCommandBufferInheritanceInfo inheritanceInfo;
		//	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		//	inheritanceInfo.renderPass = renderPass;
		//	inheritanceInfo.framebuffer = frameBuffers[i];
		//	inheritanceInfo.subpass = 0;
		//	inheritanceInfo.pNext = nullptr;
		//	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
		//	inheritanceInfo.queryFlags = 0;
		//	inheritanceInfo.pipelineStatistics = 0;

		//	VkCommandBufferBeginInfo beginInfo = {};
		//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		//	beginInfo.pInheritanceInfo = &inheritanceInfo; // Optional
		//	if (vkBeginCommandBuffer(afterPushCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
		//		throw std::runtime_error("failed to begin recording after push secondary command buffer!");
		//	}
		//	vkCmdBindPipeline(afterPushCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		//	vkCmdBindDescriptorSets(afterPushCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[0], 0, nullptr);
		//	vkCmdDraw(afterPushCommandBuffers[i], 6, 1, 0, 0);
		//	if (vkEndCommandBuffer(afterPushCommandBuffers[i]) != VK_SUCCESS) {
		//		throw std::runtime_error("failed to record after push secondary command buffer!");
		//	}
		//}
		//VkCommandBuffer arr[] = { pushCommandBuffers[i],afterPushCommandBuffers[i] };
		vkCmdBindPipeline(primaryCommandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdPushConstants(primaryCommandBuffers[index], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float), &theta);
		vkCmdBindDescriptorSets(primaryCommandBuffers[index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[0], 0, nullptr);
		vkCmdDraw(primaryCommandBuffers[index], 6, 1, 0, 0);
			
		//vkCmdExecuteCommands(primaryCommandBuffers[index],2, arr);
		vkCmdEndRenderPass(primaryCommandBuffers[index]);

		if (vkEndCommandBuffer(primaryCommandBuffers[index]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void createSwapChainObjects() {
		VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		vk->createSwapChain(surface, sc,extent);
		vk->createSwapChainImageViews(sc);
		vk->createRenderPass(renderPass, sc.format, msaaSamples, true, true, false, true);
		std::vector<VkPushConstantRange> push(1,{ VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(float) });
		vk->createGraphicsPipeline(pipeline,
			pipelineLayout,
			renderPass,
			vertexShader, fragmentShader,
			constants::NullVertexDescriptions,
			&push,
			descriptorSetLayout,
			sc.extent,
			msaaSamples
		);
		frameBuffers = vk->createFramebuffers(renderPass, sc,nullptr,nullptr);
		primaryCommandBuffers = vk->createCommandBuffers(commandPool, frameBuffers.size(), true);
		prePushCommandBuffers = vk->createCommandBuffers(commandPool, frameBuffers.size(), false);
		afterPushCommandBuffers = vk->createCommandBuffers(commandPool, frameBuffers.size(),false);
		pushCommandBuffers = vk->createCommandBuffers(secondaryCommandPool, frameBuffers.size(),false);
		vk->createDescriptorPool(descriptorPool, 1, 1, 1);
		createUniformBuffers();
		createDescriptorSets();
		writeDescriptorSets();
		vk->createSyncObjects(syncObjects, sc.imageCount);
	}

	void cleanupSwapChain() {
		for (int i = 0; i < uniformBuffers.size(); i++)
			if (uniformBuffers[i].buffer != VK_NULL_HANDLE)  vk->destroyBufferBundle(uniformBuffers[i]);
		if (primaryCommandBuffers.size() != 0) { vkFreeCommandBuffers(vk->device, commandPool, primaryCommandBuffers.size(), primaryCommandBuffers.data()); primaryCommandBuffers.resize(0); };
		if (prePushCommandBuffers.size() != 0) { vkFreeCommandBuffers(vk->device, commandPool, prePushCommandBuffers.size(), prePushCommandBuffers.data()); prePushCommandBuffers.resize(0); };
		if (afterPushCommandBuffers.size() != 0) { vkFreeCommandBuffers(vk->device, commandPool, afterPushCommandBuffers.size(), afterPushCommandBuffers.data()); afterPushCommandBuffers.resize(0); };
		if (pushCommandBuffers.size() != 0) { vkFreeCommandBuffers(vk->device, secondaryCommandPool, pushCommandBuffers.size(), pushCommandBuffers.data()); pushCommandBuffers.resize(0); };
		for (int i = 0; i < frameBuffers.size(); i++)
			if (frameBuffers[i] != VK_NULL_HANDLE) { vkDestroyFramebuffer(vk->device, frameBuffers[i], nullptr); frameBuffers[i] = VK_NULL_HANDLE; }
		if (pipeline != VK_NULL_HANDLE) { vkDestroyPipeline(vk->device, pipeline, nullptr); pipeline = VK_NULL_HANDLE; }
		if (pipelineLayout != VK_NULL_HANDLE) { vkDestroyPipelineLayout(vk->device, pipelineLayout, nullptr); pipelineLayout = VK_NULL_HANDLE; }
		if (renderPass != VK_NULL_HANDLE) { vkDestroyRenderPass(vk->device, renderPass, nullptr); renderPass = VK_NULL_HANDLE; }
		for (int i = 0; i < sc.imageViews.size(); i++) 
			if (sc.imageViews[i] != VK_NULL_HANDLE) { vkDestroyImageView(vk->device, sc.imageViews[i], nullptr); sc.imageViews[i] = VK_NULL_HANDLE; }
		if (sc.swapChain != VK_NULL_HANDLE) { vkDestroySwapchainKHR(vk->device, sc.swapChain, nullptr); sc.swapChain = VK_NULL_HANDLE; }
		if (descriptorPool != VK_NULL_HANDLE) { vkDestroyDescriptorPool(vk->device, descriptorPool, nullptr); descriptorPool = VK_NULL_HANDLE; }
		vk->cleanupSyncObjects(syncObjects);
	}

	void cleanup() {
		vkDeviceWaitIdle(vk->device);


		if (descriptorSetLayout != VK_NULL_HANDLE) { vkDestroyDescriptorSetLayout(vk->device, descriptorSetLayout, nullptr); descriptorSetLayout = VK_NULL_HANDLE; }
		if (vertexShader != VK_NULL_HANDLE) { vkDestroyShaderModule(vk->device, vertexShader, nullptr); vertexShader = VK_NULL_HANDLE; }
		if (fragmentShader != VK_NULL_HANDLE) { vkDestroyShaderModule(vk->device, fragmentShader, nullptr); fragmentShader = VK_NULL_HANDLE; }
		cleanupSwapChain();
		vk->cleanupSampledImage(texture);
		if (commandPool != VK_NULL_HANDLE) { vkDestroyCommandPool(vk->device, commandPool, nullptr); commandPool = VK_NULL_HANDLE; }
		if (secondaryCommandPool != VK_NULL_HANDLE) { vkDestroyCommandPool(vk->device, secondaryCommandPool, nullptr); secondaryCommandPool = VK_NULL_HANDLE; }
		if (surface != VK_NULL_HANDLE) { vk->destroySurface(surface); surface = VK_NULL_HANDLE; }
		vk->shutdownVulkanEngine();
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void recreateSwapChain() {
		int width = 0, height = 0;
		// Check the special case where the window is minimized and the framebuffer size is zero
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
		swapChainOutdated = false;
		
		vkDeviceWaitIdle(vk->device);
		cleanupSwapChain();
		createSwapChainObjects();
	}

	void drawPresentFrame() {

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

		writeCommandBuffer(imageIndex, angle);

		std::vector<VkCommandBuffer> submitCommmandBuffers = { primaryCommandBuffers[imageIndex] };
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


	void mainLoop() {
		// Event Handler
		static auto lastTime = std::chrono::high_resolution_clock::now();
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawPresentFrame();
			if (framebufferResized || swapChainOutdated) {
				framebufferResized = false;
				recreateSwapChain();
			}
			auto currentTime = std::chrono::high_resolution_clock::now();
			angle = (std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count());
		}
	}

public:
	void setWindowSize(int w, int h) {
		width = w;
		height = h;
	}
	void run() {
		init();
		mainLoop();
		cleanup();
	}
};
int main(int argc, char** argv) {
	VulkanEngine vk;
	PeekaBooApp app(&vk);
	try {
		app.setWindowSize(800, 800);
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;

}