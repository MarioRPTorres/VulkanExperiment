#include "vulkan_imgui.h"

// Forward Declarations
void createImguiRenderPass(VkDevice device, VkFormat format, VkRenderPass& renderPass);
void createImguiCommandBuffers(VkDevice device, uint32_t swapChainImageCount, VulkanImguiDeviceObjects& imObj);
void createImguiFrameBuffers(VkDevice device, SwapChainDetails sc, VulkanImguiDeviceObjects& imObj);

void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

void initImgui(VulkanEngine* vk,VulkanImguiDeviceObjects& imObj) {
	VulkanBackEndData bd = vk->getBackEndData();
	SwapChainDetails sc = vk->getSwapChainDetails();
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
	ImGui_ImplGlfw_InitForVulkan(bd.window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = bd.instance;
	init_info.PhysicalDevice = bd.physicalDevice;
	init_info.Device = bd.device;
	init_info.QueueFamily = 0;
	init_info.Queue = bd.graphicsQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = imObj.descriptorPool;
	init_info.Allocator = nullptr;
	init_info.MinImageCount = sc.minImageCount;
	init_info.ImageCount = sc.imageCount;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, imObj.renderPass);

	VkCommandBuffer command_buffer = vk->beginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
	vk->endSingleTimeCommands(command_buffer);
}

void createImguiDeviceObjects(VulkanEngine* vk, VulkanImguiDeviceObjects& imObj) {
	VulkanBackEndData bd = vk->getBackEndData();
	SwapChainDetails sc = vk->getSwapChainDetails();



	// **************** Descriptor Pool ************************
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
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolInfo.maxSets = 1000 * pool_sizes_count;
	descriptorPoolInfo.poolSizeCount = pool_sizes_count;
	descriptorPoolInfo.pPoolSizes = pool_sizes;

	if (vkCreateDescriptorPool(bd.device, &descriptorPoolInfo, nullptr, &imObj.descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create imgui descriptor pool!");
	}



	// **************** Command Pool ************************

	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo .queueFamilyIndex = bd.graphicsQueueFamily;
	commandPoolInfo .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allow command buffers to be rerecorded 
	//	individually, without this flag they all have to be reset together


	// Here there is a choice for the Allocator function
	if (vkCreateCommandPool(bd.device, &commandPoolInfo, nullptr, &imObj.commandPool ) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}

	createImguiCommandBuffers(bd.device, sc.imageCount, imObj);
	createImguiRenderPass(bd.device, sc.format, imObj.renderPass);
	createImguiFrameBuffers(bd.device, sc, imObj);
}

void createImguiRenderPass(VkDevice device, VkFormat format, VkRenderPass& renderPass) {
	
	// ****************  Imgui Render Pass **************
	// Color Attachment creation
	VkAttachmentDescription imguiColorAttachment = {};
	imguiColorAttachment.format = format;
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
	if (vkCreateRenderPass(device, &imguiRenderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create imgui render pass!");
	}
}

void createImguiCommandBuffers(VkDevice device, uint32_t swapChainImageCount, VulkanImguiDeviceObjects& imObj) {
	// **************** Command Buffers ************************

	imObj.commandBuffers.resize(swapChainImageCount);
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = imObj.commandPool;
	//• VK_COMMAND_BUFFER_LEVEL_PRIMARY : Can be submitted to a queue for execution, but cannot be called from other command buffers.
	//• VK_COMMAND_BUFFER_LEVEL_SECONDARY : Cannot be submitted directly, but can be called from primary command buffers.
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)imObj.commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, imObj.commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate imgui command buffers!");
	}
}

void createImguiFrameBuffers(VkDevice device, SwapChainDetails sc,VulkanImguiDeviceObjects& imObj) {
	uint32_t imageCount = static_cast<uint32_t>(sc.imageViews.size());
	imObj.frameBuffers.resize(imageCount);

	for (int i = 0; i < imageCount; i++) {
		VkImageView attachment = sc.imageViews[i];

		VkFramebufferCreateInfo frameBufferInfo = {};
		frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferInfo.renderPass = imObj.renderPass;
		frameBufferInfo.attachmentCount = 1;
		frameBufferInfo.pAttachments = &attachment;
		frameBufferInfo.width = sc.extent.width;
		frameBufferInfo.height = sc.extent.height;
		frameBufferInfo.layers = 1;

		// Here there is a choice for the Allocator function
		if (vkCreateFramebuffer(device, &frameBufferInfo, nullptr, &imObj.frameBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create imgui framebuffer!");
		}
	}
}

void cleanupImguiObjects(VkDevice device, VulkanImguiDeviceObjects& imObj) {
	cleanupImguiSwapChainObjects(device, imObj);

	// Resources to destroy when the program ends
	vkDestroyDescriptorPool(device, imObj.descriptorPool, nullptr);
	vkDestroyCommandPool(device, imObj.commandPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void cleanupImguiSwapChainObjects(VkDevice device, VulkanImguiDeviceObjects& imObj) {
	for (size_t i = 0; i < imObj.frameBuffers.size(); i++) {
		vkDestroyFramebuffer(device, imObj.frameBuffers[i], nullptr);
	}
	vkFreeCommandBuffers(device, imObj.commandPool, static_cast<uint32_t>(imObj.commandBuffers.size()), imObj.commandBuffers.data());
	vkDestroyRenderPass(device, imObj.renderPass, nullptr);
}

void recreateImguiSwapChainObjects(VulkanEngine* vk, VulkanImguiDeviceObjects& imObj) {
	VulkanBackEndData bd = vk->getBackEndData();
	SwapChainDetails sc = vk->getSwapChainDetails();
	
	ImGui_ImplVulkan_SetMinImageCount(sc.minImageCount);
	createImguiCommandBuffers(bd.device, sc.imageCount, imObj);
	createImguiRenderPass(bd.device, sc.format, imObj.renderPass);
	createImguiFrameBuffers(bd.device, sc, imObj);
}