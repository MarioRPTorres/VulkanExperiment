#include "vulkan_imgui.h"
#include "vulkan_descriptors.h"
#include "imgui_impl_glfw.h"

// glsl_shader.vert, compiled with:
// # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
/*
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

void main()
{
	Out.Color = aColor;
	Out.UV = aUV;
	gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
*/
static shaderCode __glsl_shader_vert_spv =
{
	0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
	0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
	0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
	0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
	0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
	0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
	0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
	0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
	0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
	0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
	0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
	0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
	0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
	0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
	0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
	0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
	0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
	0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
	0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
	0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
	0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
	0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
	0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
	0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
	0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
	0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
	0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
	0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
	0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
	0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
	0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
	0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
	0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
	0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
	0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
	0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
	0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
	0x0000002d,0x0000002c,0x000100fd,0x00010038
};

// glsl_shader.frag, compiled with:
// # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag
/*
#version 450 core
layout(location = 0) out vec4 fColor;
layout(set=0, binding=0) uniform sampler2D sTexture;
layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
void main()
{
	fColor = In.Color * texture(sTexture, In.UV.st);
}
*/
static shaderCode __glsl_shader_frag_spv =
{
	0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
	0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
	0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
	0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
	0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
	0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
	0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
	0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
	0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
	0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
	0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
	0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
	0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
	0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
	0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
	0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
	0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
	0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
	0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
	0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
	0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
	0x00010038
};


///***************************************************************************************************//
static VkEImgui_Backend* VkEImgui_GetBackendData();
static void VkEImgui_CreateDescriptorSetLayout(VkEImgui_Backend* bd);
static void VkEImgui_CreatePipelineLayout(VkEImgui_Backend* bd);
static void VkEImgui_CreatePipeline(VkEImgui_Backend* bd);


static uint32_t ImGui_ImplVulkan_MemoryType(VkMemoryPropertyFlags properties, uint32_t type_bits)
{
	VkEImgui_Backend* bd = VkEImgui_GetBackendData();
	VulkanBackEndData vkBd = bd->engine->getBackEndData();

	VkPhysicalDeviceMemoryProperties prop;
	vkGetPhysicalDeviceMemoryProperties(vkBd.physicalDevice, &prop);
	for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
		if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i))
			return i;
	return 0xFFFFFFFF; // Unable to find memoryType
}

void VkE_Imgui_NewFrame() {
	VkEImgui_Backend* bd = VkEImgui_GetBackendData();
	IM_ASSERT(bd != NULL && "Did you call ImGui_ImplVulkan_Init()?");
	IM_UNUSED(bd);
}

static void CreateOrResizeBuffer(VkBuffer& buffer, VkDeviceMemory& buffer_memory, VkDeviceSize& p_buffer_size, size_t new_size, VkBufferUsageFlagBits usage)
{
	VkEImgui_Backend* bd = VkEImgui_GetBackendData();
	VulkanBackEndData vkBd = bd->engine->getBackEndData();

	VkResult err;
	if (buffer != VK_NULL_HANDLE)
		vkDestroyBuffer(vkBd.device, buffer, nullptr);
	if (buffer_memory != VK_NULL_HANDLE)
		vkFreeMemory(vkBd.device, buffer_memory, nullptr);

	VkDeviceSize vertex_buffer_size_aligned = ((new_size - 1) / bd->BufferMemoryAlignment + 1) * bd->BufferMemoryAlignment;
	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = vertex_buffer_size_aligned;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	err = vkCreateBuffer(vkBd.device, &buffer_info, nullptr, &buffer);
	check_vk_result(err);

	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(vkBd.device, buffer, &req);
	bd->BufferMemoryAlignment = (bd->BufferMemoryAlignment > req.alignment) ? bd->BufferMemoryAlignment : req.alignment;
	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = req.size;
	alloc_info.memoryTypeIndex = ImGui_ImplVulkan_MemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
	err = vkAllocateMemory(vkBd.device, &alloc_info, nullptr, &buffer_memory);
	check_vk_result(err);

	err = vkBindBufferMemory(vkBd.device, buffer, buffer_memory, 0);
	check_vk_result(err);
	p_buffer_size = req.size;
}

static void VkEImgui_SetupRenderState(ImDrawData* draw_data, VkPipeline pipeline, VkCommandBuffer command_buffer, VkEImgui_vertexBuffers* rb, int fb_width, int fb_height)
{
	VkEImgui_Backend* bd = VkEImgui_GetBackendData();

	// Bind pipeline:
	{
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	// Bind Vertex And Index Buffer:
	if (draw_data->TotalVtxCount > 0)
	{
		VkBuffer vertex_buffers[1] = { rb->vertex.buffer };
		VkDeviceSize vertex_offset[1] = { 0 };
		vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, vertex_offset);
		vkCmdBindIndexBuffer(command_buffer, rb->index.buffer, 0, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
	}

	// Setup viewport:
	{
		VkViewport viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = (float)fb_width;
		viewport.height = (float)fb_height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	}

	// Setup scale and translation:
	// Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
	{
		float scale[2];
		scale[0] = 2.0f / draw_data->DisplaySize.x;
		scale[1] = 2.0f / draw_data->DisplaySize.y;
		float translate[2];
		translate[0] = -1.0f - draw_data->DisplayPos.x * scale[0];
		translate[1] = -1.0f - draw_data->DisplayPos.y * scale[1];
		vkCmdPushConstants(command_buffer, bd->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 0, sizeof(float) * 2, scale);
		vkCmdPushConstants(command_buffer, bd->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(float) * 2, sizeof(float) * 2, translate);
	}
}

// Render function
void VkEImgui_RenderDrawData(void* imgui_draw_data, VkCommandBuffer command_buffer, VkPipeline pipeline)
{
	ImDrawData* draw_data = static_cast<ImDrawData*> (imgui_draw_data);
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0)
		return;

	VkEImgui_Backend* bd = VkEImgui_GetBackendData();
	VulkanBackEndData vkBd = bd->engine->getBackEndData();
	if (pipeline == VK_NULL_HANDLE)
		pipeline = bd->pipeline;

	// Allocate array to store enough vertex/index buffers. Each unique viewport gets its own storage.
	VkEImgui_Viewport* wrb = (VkEImgui_Viewport*)draw_data->OwnerViewport->RendererUserData;
	IM_ASSERT(wrb != NULL);
	if (wrb->vertexBuffers.size() == 0) {
		wrb->imageIndex = 0;
		wrb->vertexBuffers.resize(wrb->sc.imageCount);
	}
	//IM_ASSERT(wrb->Count == bd->imageCount);
	//wrb->imageIndex = (wrb->imageIndex + 1) % wrb->sc.imageCount;
	VkEImgui_vertexBuffers* rb = &wrb->vertexBuffers[wrb->imageIndex];

	if (draw_data->TotalVtxCount > 0) {
		// Create or resize the vertex/index buffers
		size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
		size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
		if (rb->vertex.buffer == VK_NULL_HANDLE || rb->vertex.size < vertex_size)
			CreateOrResizeBuffer(rb->vertex.buffer, rb->vertex.memory, rb->vertex.size, vertex_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		if (rb->index.buffer == VK_NULL_HANDLE || rb->index.size < index_size)
			CreateOrResizeBuffer(rb->index.buffer, rb->index.memory, rb->index.size, vertex_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		// Upload vertex/index data into a single contiguous GPU buffer
		ImDrawVert* vtx_dst = NULL;
		ImDrawIdx* idx_dst = NULL;
		VkResult err = vkMapMemory(vkBd.device, rb->vertex.memory, 0, rb->vertex.size, 0, (void**)(&vtx_dst));
		check_vk_result(err);
		err = vkMapMemory(vkBd.device, rb->index.memory, 0, rb->index.size, 0, (void**)(&idx_dst));
		check_vk_result(err);
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtx_dst += cmd_list->VtxBuffer.Size;
			idx_dst += cmd_list->IdxBuffer.Size;
		}
		VkMappedMemoryRange range[2] = {};
		range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range[0].memory = rb->vertex.memory;
		range[0].size = VK_WHOLE_SIZE;
		range[1].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range[1].memory = rb->index.memory;
		range[1].size = VK_WHOLE_SIZE;
		err = vkFlushMappedMemoryRanges(vkBd.device, 2, range);
		check_vk_result(err);
		vkUnmapMemory(vkBd.device, rb->vertex.memory);
		vkUnmapMemory(vkBd.device, rb->index.memory);
	}

	// Setup desired Vulkan state
	VkEImgui_SetupRenderState(draw_data, pipeline, command_buffer, rb, fb_width, fb_height);

	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	// Render command lists
	// (Because we merged all buffers into a single one, we maintain our own offset into them)
	int global_vtx_offset = 0;
	int global_idx_offset = 0;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback != NULL)
			{
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					VkEImgui_SetupRenderState(draw_data, pipeline, command_buffer, rb, fb_width, fb_height);
				else
					pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				// Project scissor/clipping rectangles into framebuffer space
				ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
				ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

				// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
				if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
				if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
				if (clip_max.x > fb_width) { clip_max.x = (float)fb_width; }
				if (clip_max.y > fb_height) { clip_max.y = (float)fb_height; }
				if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
					continue;

				// Apply scissor/clipping rectangle
				VkRect2D scissor;
				scissor.offset.x = (int32_t)(clip_min.x);
				scissor.offset.y = (int32_t)(clip_min.y);
				scissor.extent.width = (uint32_t)(clip_max.x - clip_min.x);
				scissor.extent.height = (uint32_t)(clip_max.y - clip_min.y);
				vkCmdSetScissor(command_buffer, 0, 1, &scissor);

				// Bind DescriptorSet with font or user texture
				VkDescriptorSet desc_set[1] = { (VkDescriptorSet)pcmd->TextureId };
				if (sizeof(ImTextureID) < sizeof(ImU64))
				{
					// We don't support texture switches if ImTextureID hasn't been redefined to be 64-bit. Do a flaky check that other textures haven't been used.
					IM_ASSERT(pcmd->TextureId == (ImTextureID)bd->fontDescriptorSet);
					desc_set[0] = bd->fontDescriptorSet;
				}
				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, bd->pipelineLayout, 0, 1, desc_set, 0, NULL);

				// Draw
				vkCmdDrawIndexed(command_buffer, pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset, 0);
			}
		}
		global_idx_offset += cmd_list->IdxBuffer.Size;
		global_vtx_offset += cmd_list->VtxBuffer.Size;
	}

	// Note: at this point both vkCmdSetViewport() and vkCmdSetScissor() have been called.
	// Our last values will leak into user/application rendering IF:
	// - Your app uses a pipeline with VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR dynamic state
	// - And you forgot to call vkCmdSetViewport() and vkCmdSetScissor() yourself to explicitely set that state.
	// If you use VK_DYNAMIC_STATE_VIEWPORT or VK_DYNAMIC_STATE_SCISSOR you are responsible for setting the values before rendering.
	// In theory we should aim to backup/restore those values but I am not sure this is possible.
	// We perform a call to vkCmdSetScissor() to set back a full viewport which is likely to fix things for 99% users but technically this is not perfect. (See github #4644)
	VkRect2D scissor = { { 0, 0 }, { (uint32_t)fb_width, (uint32_t)fb_height } };
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

// ***VulkanImgui***

void VkEImgui_CreateViewportSwapBufferObjects(VkEImgui_Viewport* vp) {

	VkEImgui_Backend* bd = VkEImgui_GetBackendData();
	VulkanEngine* vk = bd->engine;
	VulkanBackEndData vBd = bd->engine->getBackEndData();

	vk->createSwapChain(vp->surface, vp->sc);
	vk->createSwapChainImageViews(vp->sc.images, vp->sc.format, vp->sc.imageViews);
	vp->renderPass = bd->engine->createRenderPass(vp->sc.format, VK_SAMPLE_COUNT_1_BIT, true, true, false, vp->ClearEnable);
	VkEImgui_CreatePipeline(bd);

	QueueFamilyIndices indices = findQueueFamilies(vBd.physicalDevice, vp->surface);
	if (indices.presentFamily.has_value) {
		vkGetDeviceQueue(vBd.device, indices.presentFamily.value, 0, &vp->presentQueue);
	}
	else {
		vp->presentQueue = vBd.graphicsQueue;
	}

	// Create Frame Buffers
	vp->frameBuffers = vk->createFramebuffers(vp->renderPass, vp->sc);
	// Create Command Buffers x
	vp->commandBuffers = vk->createCommandBuffers(vp->commandPool, vp->sc.imageCount);
	// Create Sync Objects
	vk->createSyncObjects(vp->syncObjects, vp->sc.imageCount);
	vp->vertexBuffers.resize(vp->sc.imageCount);
}

static void VkEImgui_CreateWindow(ImGuiViewport* viewport)
{
	VkEImgui_Backend* bd = VkEImgui_GetBackendData();
	VulkanEngine* vk = bd->engine;
	VulkanBackEndData vkBd = vk->getBackEndData();
	bd->viewports.push_back(new VkEImgui_Viewport());
	VkEImgui_Viewport* vp = bd->viewports.back();

	viewport->RendererUserData = (void*)vp;
	vp->ClearEnable = (viewport->Flags & ImGuiViewportFlags_NoRendererClear) ? false : true;
	vp->WindowOwned = true;
	// ImGui_ImplVulkanH_Window* wd = &vd->Window;
	// ImGui_ImplVulkan_InitInfo* v = &bd->VulkanInitInfo;

	// create the shader modules 
	if (bd->ShaderModuleVert == VK_NULL_HANDLE) bd->ShaderModuleVert = vk->createShaderModule(__glsl_shader_vert_spv);
	if (bd->ShaderModuleFrag == VK_NULL_HANDLE) bd->ShaderModuleFrag = vk->createShaderModule(__glsl_shader_frag_spv);
	VkEImgui_CreateDescriptorSetLayout(bd);
	VkEImgui_CreatePipelineLayout(bd);
	// Create Command pool x
	vk->createCommandPool(vp->commandPool, vkBd.graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	// Create surface
	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
	VkResult err = (VkResult)platform_io.Platform_CreateVkSurface(viewport, (ImU64)vkBd.instance, nullptr, (ImU64*)&vp->surface);
	check_vk_result(err);

	VkEImgui_CreateViewportSwapBufferObjects(vp);

	// Create SwapChain, RenderPass, Framebuffer, etc.
	//ImGui_ImplVulkanH_CreateOrResizeWindow(v->Instance, v->PhysicalDevice, v->Device, wd, v->QueueFamily, v->Allocator, (int)viewport->Size.x, (int)viewport->Size.y, v->MinImageCount);
}


static void VkEImgui_cleanupViewportSwapChain(VkEImgui_Viewport* vp) {
	VkEImgui_Backend *bd = VkEImgui_GetBackendData();
	VulkanEngine* vk = bd->engine;
	VulkanBackEndData vkBd = vk->getBackEndData();

	std::vector<VkFence>& inFlightFences = vp->syncObjects.inFlightFences;
	vkWaitForFences(vkBd.device,  inFlightFences.size(), inFlightFences.data(), VK_TRUE, UINT64_MAX);

	for (size_t i = 0; i < vp->frameBuffers.size(); i++) {
		vkDestroyFramebuffer(vkBd.device, vp->frameBuffers[i], nullptr);
		vkDestroyImageView(vkBd.device, vp->sc.imageViews[i], nullptr);
	}

	vkFreeCommandBuffers(vkBd.device, vp->commandPool, static_cast<uint32_t>(vp->commandBuffers.size()), vp->commandBuffers.data());
	vkDestroyPipeline(vkBd.device, vp->pipeline, nullptr);
	vkDestroyRenderPass(vkBd.device, vp->renderPass, nullptr);
	vkDestroySwapchainKHR(vkBd.device, vp->sc.swapChain, nullptr);
	vk->cleanupSyncObjects(vp->syncObjects);
}

static void VkEImgui_DestroyWindow(ImGuiViewport* viewport) {
	VkEImgui_Backend* bd = VkEImgui_GetBackendData();
	if (bd == NULL) return;
	
	VulkanBackEndData vkBd = bd->engine->getBackEndData();
	std::vector<VkEImgui_Viewport*> vps = bd->viewports;
	VkEImgui_Viewport* vp = (VkEImgui_Viewport*)viewport->RendererUserData;
	
	if (vp != NULL) {
		VkEImgui_cleanupViewportSwapChain(vp);

		vkDestroyCommandPool(vkBd.device, vp->commandPool, nullptr);
		for (size_t i = 0; i < vp->vertexBuffers.size(); i++) {
			BufferBundle* vertex = &vp->vertexBuffers[i].vertex;
			BufferBundle* index = &vp->vertexBuffers[i].index;
			if (vertex->buffer) { vkDestroyBuffer(vkBd.device, vertex->buffer, nullptr); vertex->buffer = VK_NULL_HANDLE; }
			if (vertex->memory) { vkFreeMemory(vkBd.device, vertex->memory, nullptr); vertex->memory = VK_NULL_HANDLE; }
			if (index->buffer) { vkDestroyBuffer(vkBd.device, index->buffer, nullptr); index->buffer = VK_NULL_HANDLE; }
			if (index->memory) { vkFreeMemory(vkBd.device, index->memory, nullptr); index->memory = VK_NULL_HANDLE; }
			vertex->size = 0;
			index->size = 0;
		}
		vkDestroySurfaceKHR(vkBd.instance, vp->surface, nullptr);

		for (int i = 0; i < vps.size(); i++) {
			if (vps[i] == vp) {
				vps.erase(vps.begin() + i);
				break;
			}
		}
		delete vp;
		//vps->erase(std::remove(vps->begin(), vps->end(), vp), vps->end());
		viewport->RendererUserData = NULL;
	}
}

static void VkEImgui_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
	VkEImgui_Backend* bd = VkEImgui_GetBackendData();
	VulkanBackEndData vkBd = bd->engine->getBackEndData();
	VkEImgui_Viewport* vp = (VkEImgui_Viewport*)viewport->RendererUserData;

	if (vp == NULL) // This is NULL for the main viewport (which is left to the user/app to handle)
		return;

	vp->swapChainOutdated = false;
	vp->ClearEnable = (viewport->Flags & ImGuiViewportFlags_NoRendererClear) ? false : true;
	vp->width = size.x;
	vp->height = size.y;

	VkEImgui_cleanupViewportSwapChain(vp);
	VkEImgui_CreateViewportSwapBufferObjects(vp);
}

static void VkEImgui_RenderWindow(ImGuiViewport* viewport, void*)
{
	VkEImgui_Backend* bd = VkEImgui_GetBackendData();
	VulkanBackEndData vkBd = bd->engine->getBackEndData();
	VkEImgui_Viewport* vp = (VkEImgui_Viewport*)viewport->RendererUserData;
	VkE_FrameSyncObjects& so = vp->syncObjects;
	VkResult err;

	uint32_t inFlightFrameIndex = vp->inFlightFrameIndex;
	vkWaitForFences(vkBd.device, 1, &so.inFlightFences[inFlightFrameIndex], VK_TRUE, UINT64_MAX);

	err = vkAcquireNextImageKHR(vkBd.device, vp->sc.swapChain, UINT64_MAX, so.imageAvailableSemaphore[inFlightFrameIndex], VK_NULL_HANDLE, &vp->imageIndex);
	
	// Using the result we can check if the swapchain is out of data and if so we need to recreate it
	if (err == VK_ERROR_OUT_OF_DATE_KHR) {
		vp->swapChainOutdated = true;
		return;
	}
	check_vk_result(err);
	
	uint32_t& imageIndex = vp->imageIndex;
	if (so.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		for (;;)
		{
			err = vkWaitForFences(vkBd.device, 1, &so.imagesInFlight[imageIndex], VK_TRUE, 100);
			if (err == VK_SUCCESS) break;
			if (err == VK_TIMEOUT) continue;
			check_vk_result(err);
		}
	}
	// Mark the image as now being in use by this frame
	so.imagesInFlight[imageIndex] = so.inFlightFences[inFlightFrameIndex];
	
	VkCommandBuffer& cb = vp->commandBuffers[imageIndex];
	err = vkResetCommandBuffer(cb, 0);
	check_vk_result(err);
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	err = vkBeginCommandBuffer(cb, &info);
	check_vk_result(err);


	VkRenderPassBeginInfo rpBeginInfo = {};
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.renderPass = vp->renderPass;
	rpBeginInfo.framebuffer = vp->frameBuffers[imageIndex];
	rpBeginInfo.renderArea.extent.width = vp->sc.extent.width;
	rpBeginInfo.renderArea.extent.height = vp->sc.extent.height;

	if (viewport->Flags & ImGuiViewportFlags_NoRendererClear) {
		rpBeginInfo.clearValueCount =  0;
		rpBeginInfo.pClearValues =  NULL ;
	}
	else if(true) {
		ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		memcpy(&vp->clearValue.color.float32[0], &clear_color, 4 * sizeof(float));
		rpBeginInfo.clearValueCount =  1;
		rpBeginInfo.pClearValues = &vp->clearValue;
	}

	vkCmdBeginRenderPass(cb, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	VkEImgui_RenderDrawData(viewport->DrawData, cb, vp->pipeline);


	vkCmdEndRenderPass(cb);
	err = vkEndCommandBuffer(cb);
	check_vk_result(err);
		
	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &so.imageAvailableSemaphore[inFlightFrameIndex];
	submitInfo.pWaitDstStageMask = &wait_stage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cb;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &so.renderFinishedSemaphore[inFlightFrameIndex];

			
	err = vkResetFences(vkBd.device, 1, &so.inFlightFences[inFlightFrameIndex]);
	check_vk_result(err);
	err = vkQueueSubmit(vkBd.graphicsQueue, 1, &submitInfo, so.inFlightFences[inFlightFrameIndex]);
	check_vk_result(err);
}

static void VkEImgui_SwapBuffers(ImGuiViewport* viewport,void*){
	VkEImgui_Backend* bd = VkEImgui_GetBackendData();
	VkEImgui_Viewport* vp = (VkEImgui_Viewport*)viewport->RendererUserData;

	if (vp->swapChainOutdated) {
		VkEImgui_SetWindowSize(viewport, ImVec2((int)viewport->Size.x, (int)viewport->Size.y));
		return;
	}
	VkResult err;
	uint32_t present_index = vp->inFlightFrameIndex;

	VkPresentInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	info.waitSemaphoreCount = 1;
	info.pWaitSemaphores = &vp->syncObjects.renderFinishedSemaphore[present_index];
	info.swapchainCount = 1;
	info.pSwapchains = &vp->sc.swapChain;
	info.pImageIndices = &vp->imageIndex;
	err = vkQueuePresentKHR(vp->presentQueue, &info);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
		VkEImgui_SetWindowSize(viewport, ImVec2((int)viewport->Size.x, (int)viewport->Size.y));
	else
		check_vk_result(err);

	vp->inFlightFrameIndex = (vp->inFlightFrameIndex + 1) % bd->maxFramesInFlight;         // This is for the next vkWaitForFences()
}

void VkEImgui_InitPlatformInterface()
{
	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		IM_ASSERT(platform_io.Platform_CreateVkSurface != NULL && "Platform needs to setup the CreateVkSurface handler.");
	platform_io.Renderer_CreateWindow = VkEImgui_CreateWindow;
	platform_io.Renderer_DestroyWindow = VkEImgui_DestroyWindow;
	platform_io.Renderer_SetWindowSize = VkEImgui_SetWindowSize;
	platform_io.Renderer_RenderWindow = VkEImgui_RenderWindow;
	platform_io.Renderer_SwapBuffers = VkEImgui_SwapBuffers;
}

void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

static VkEImgui_Backend* VkEImgui_GetBackendData()
{
	return ImGui::GetCurrentContext() ? (VkEImgui_Backend*)ImGui::GetIO().BackendRendererUserData : NULL;
}

static void VkEImgui_CreateDescriptorSetLayout(VkEImgui_Backend* bd) {
	if (bd->descriptorSetLayout != VK_NULL_HANDLE)
		return;

	VkDevice device = bd->engine->getBackEndData().device;

	IM_ASSERT(bd->fontSImage.sampler != VK_NULL_HANDLE && "Need to create a vulkan image sampler. FontImage Sampler is not set!");

	VkSampler sampler[1] = { bd->fontSImage.sampler };
	VkDescriptorSetLayoutBinding binding[1] = {};
	binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding[0].descriptorCount = 1;
	binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	binding[0].pImmutableSamplers = sampler;
	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = 1;
	info.pBindings = binding;
	VkResult err = vkCreateDescriptorSetLayout(device, &info, nullptr, &bd->descriptorSetLayout);
	check_vk_result(err);
}

static void VkEImgui_CreatePipelineLayout(VkEImgui_Backend* bd)
{
	if (bd->pipelineLayout != VK_NULL_HANDLE)
		return;

	VkDevice device = bd->engine->getBackEndData().device;

	// Constants: we are using 'vec2 offset' and 'vec2 scale' instead of a full 3d projection matrix
	VkEImgui_CreateDescriptorSetLayout(bd);
	VkPushConstantRange push_constants[1] = {};
	push_constants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	push_constants[0].offset = sizeof(float) * 0;
	push_constants[0].size = sizeof(float) * 4;
	VkDescriptorSetLayout set_layout[1] = { bd->descriptorSetLayout };
	VkPipelineLayoutCreateInfo layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_info.setLayoutCount = 1;
	layout_info.pSetLayouts = set_layout;
	layout_info.pushConstantRangeCount = 1;
	layout_info.pPushConstantRanges = push_constants;
	VkResult  err = vkCreatePipelineLayout(device, &layout_info, bd->allocator, &bd->pipelineLayout);
	check_vk_result(err);
}

static void VkEImgui_CreatePipeline(VkEImgui_Backend* bd)
{
	if (bd->pipeline != VK_NULL_HANDLE)
		return;

	VulkanBackEndData vk = bd->engine->getBackEndData();

	VkPipelineShaderStageCreateInfo stage[2] = {};
	stage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stage[0].module = bd->ShaderModuleVert;
	stage[0].pName = "main";
	stage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stage[1].module = bd->ShaderModuleFrag;
	stage[1].pName = "main";

	VkVertexInputBindingDescription binding_desc[1] = {};
	binding_desc[0].stride = sizeof(ImDrawVert);
	binding_desc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription attribute_desc[3] = {};
	attribute_desc[0].location = 0;
	attribute_desc[0].binding = binding_desc[0].binding;
	attribute_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
	attribute_desc[0].offset = IM_OFFSETOF(ImDrawVert, pos);
	attribute_desc[1].location = 1;
	attribute_desc[1].binding = binding_desc[0].binding;
	attribute_desc[1].format = VK_FORMAT_R32G32_SFLOAT;
	attribute_desc[1].offset = IM_OFFSETOF(ImDrawVert, uv);
	attribute_desc[2].location = 2;
	attribute_desc[2].binding = binding_desc[0].binding;
	attribute_desc[2].format = VK_FORMAT_R8G8B8A8_UNORM;
	attribute_desc[2].offset = IM_OFFSETOF(ImDrawVert, col);

	VkPipelineVertexInputStateCreateInfo vertex_info = {};
	vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_info.vertexBindingDescriptionCount = 1;
	vertex_info.pVertexBindingDescriptions = binding_desc;
	vertex_info.vertexAttributeDescriptionCount = 3;
	vertex_info.pVertexAttributeDescriptions = attribute_desc;

	VkPipelineInputAssemblyStateCreateInfo ia_info = {};
	ia_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ia_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineViewportStateCreateInfo viewport_info = {};
	viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_info.viewportCount = 1;
	viewport_info.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo raster_info = {};
	raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	raster_info.polygonMode = VK_POLYGON_MODE_FILL;
	raster_info.cullMode = VK_CULL_MODE_NONE;  //X
	raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	raster_info.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo ms_info = {};
	ms_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ms_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; //X

	VkPipelineColorBlendAttachmentState color_attachment[1] = {};//X
	color_attachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_attachment[0].blendEnable = VK_TRUE;
	color_attachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_attachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_attachment[0].colorBlendOp = VK_BLEND_OP_ADD;
	color_attachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_attachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_attachment[0].alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineDepthStencilStateCreateInfo depth_info = {};//X
	depth_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

	VkPipelineColorBlendStateCreateInfo blend_info = {};
	blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blend_info.attachmentCount = 1;
	blend_info.pAttachments = color_attachment;

	// Viewport and scissor are dynamic states which means they are specified at draw time
	VkDynamicState dynamic_states[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = (uint32_t)IM_ARRAYSIZE(dynamic_states);
	dynamic_state.pDynamicStates = dynamic_states;

	VkEImgui_CreatePipelineLayout(bd);

	VkGraphicsPipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	info.flags = bd->pipelineCreateFlags;
	info.stageCount = 2;
	info.pStages = stage;
	info.pVertexInputState = &vertex_info;
	info.pInputAssemblyState = &ia_info;
	info.pViewportState = &viewport_info;
	info.pRasterizationState = &raster_info;
	info.pMultisampleState = &ms_info;
	info.pDepthStencilState = &depth_info;
	info.pColorBlendState = &blend_info;
	info.pDynamicState = &dynamic_state;
	info.layout = bd->pipelineLayout;
	info.renderPass = bd->renderPass;
	info.subpass = 0;
	VkResult err = vkCreateGraphicsPipelines(vk.device, bd->pipelineCache, 1, &info, bd->allocator, &bd->pipeline);
	check_vk_result(err);
}

void VkEImgui_setupBackEnd(VkEImgui_Backend& bd, VulkanEngine* vk, uint32_t minImageCount, uint32_t imageCount, uint32_t maxFramesInFlight) 
{
	bd.engine = vk;
	bd.minImageCount = minImageCount;
	bd.imageCount = imageCount;
	bd.maxFramesInFlight = maxFramesInFlight;
}

void VkEImgui_createBackEndObjects(VulkanEngine* vk, VkEImgui_Backend& imBd,VkEImgui_DeviceObjectsInfo info) {
	VulkanBackEndData vkBackend = imBd.engine->getBackEndData();
	VkE_SwapChain* sc = imBd.engine->getSwapChainDetails();

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

	if (vkCreateDescriptorPool(vkBackend.device, &descriptorPoolInfo, nullptr, &imBd.descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create imgui descriptor pool!");
	}


	//	individually, without this flag they all have to be reset together
	vk->createCommandPool(imBd.commandPool, vkBackend.graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	// Create core objects
	//ImGui_ImplVulkan_CreateDeviceObjects(vk,bd);


	if (!imBd.ShaderModuleVert) imBd.ShaderModuleVert = vk->createShaderModule(__glsl_shader_vert_spv);
	if (!imBd.ShaderModuleFrag) imBd.ShaderModuleFrag = vk->createShaderModule(__glsl_shader_frag_spv);
	//VkEImgui_CreateDescriptorSetLayout(&imBd);
	//VkEImgui_CreatePipelineLayout(&imBd);
	//VkEImgui_CreatePipeline(&imBd);

	imBd.commandBuffers = vk->createCommandBuffers(imBd.commandPool, sc->imageCount);
	imBd.renderPass = vk->createRenderPass(sc->format, VK_SAMPLE_COUNT_1_BIT, info.firstPass, true,false,info.firstPass);
	imBd.frameBuffers = vk->createFramebuffers(imBd.renderPass, *sc);
}

void VkEImgui_init(VulkanEngine* vk, VkEImgui_Backend& imBd) {
	IM_ASSERT(imBd.engine != nullptr && "Invalid VulkanEngine. Was VkEImgui_setupBackEnd called?");
	IM_ASSERT(imBd.minImageCount >= 2 && "Invalid minimum image count. Was VkEImgui_setupBackEnd called?");
	IM_ASSERT(imBd.imageCount >= imBd.minImageCount && "Image count lower than minimum image count");
	//IM_ASSERT(imBd.descriptorPool != VK_NULL_HANDLE);
	//IM_ASSERT(imBd.descriptorSetLayout != VK_NULL_HANDLE);
	//IM_ASSERT(imBd.pipelineLayout != VK_NULL_HANDLE);
	//IM_ASSERT(imBd.pipeline != VK_NULL_HANDLE);
	//IM_ASSERT(imBd.renderPass != VK_NULL_HANDLE);
	VulkanBackEndData vkBackend = imBd.engine->getBackEndData();
	VkE_SwapChain* sc = imBd.engine->getSwapChainDetails();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	// This feature causes imgui to create new graphic pipelines and render passes that are incompatible 
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
	ImGui_ImplGlfw_InitForVulkan(vkBackend.window, true);
	//ImGui_ImplVulkan_InitInfo init_info = {};
	//init_info.Instance = vkBackend.instance;
	//init_info.PhysicalDevice = vkBackend.physicalDevice;
	//init_info.Device = vkBackend.device;
	//init_info.QueueFamily = 0;
	//init_info.Queue = vkBackend.graphicsQueue;
	//init_info.PipelineCache = VK_NULL_HANDLE;
	//init_info.Allocator = nullptr;
	//init_info.CheckVkResultFn = check_vk_result;

	//IM_ASSERT(g_FunctionsLoaded && "Need to call ImGui_ImplVulkan_LoadFunctions() if IMGUI_IMPL_VULKAN_NO_PROTOTYPES or VK_NO_PROTOTYPES are set!");

	IM_ASSERT(io.BackendRendererUserData == NULL && "Already initialized a renderer backend!");

	// Setup backend capabilities flags
	io.BackendRendererUserData = (void*)&imBd;
	io.BackendRendererName = "VulkanEngineImgui";
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
	io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)


	//IM_ASSERT(init_info.Instance != VK_NULL_HANDLE);
	//IM_ASSERT(init_info.PhysicalDevice != VK_NULL_HANDLE);
	//IM_ASSERT(init_info.Device != VK_NULL_HANDLE);
	//IM_ASSERT(init_info.Queue != VK_NULL_HANDLE);


	imBd.mainViewport.sc = *sc;
	// Our render function expect RendererUserData to be storing the window render buffer we need (for the main viewport we won't use ->Window)
	ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	main_viewport->RendererUserData = &imBd.mainViewport;

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		VkEImgui_InitPlatformInterface();//ImGui_ImplVulkan_InitPlatformInterface();

	// Create Fonts Texture
	unsigned char* pixels;
	int width, height;
	io.Fonts->AddFontFromFileTTF("./textures/ostrich-regular.ttf", 18.0f);
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	imBd.engine->createSampledImage(imBd.fontSImage, width, height, 4, (char*)pixels, 1, VK_SAMPLE_COUNT_1_BIT);
	
	VkEImgui_CreatePipeline(&imBd);
	
	// Create the Descriptor Set for font:
	imBd.fontDescriptorSet = createSingleImageDecriptorSet(imBd.engine, imBd.descriptorPool, imBd.descriptorSetLayout, imBd.fontSImage);
	io.Fonts->SetTexID((ImTextureID)imBd.fontDescriptorSet);
}


void VkEImgui_addDefaultFont(VkEImgui_Backend& imBd) {
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Create Fonts Texture
	unsigned char* pixels;
	int width, height;
	//io.Fonts->AddFontFromFileTTF("./textures/ostrich-regular.ttf", 18.0f);
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	imBd.engine->createSampledImage(imBd.fontSImage, width, height, 4, (char*)pixels, 1, VK_SAMPLE_COUNT_1_BIT);
	// Create the Descriptor Set for font:
	imBd.fontDescriptorSet = createSingleImageDecriptorSet(imBd.engine, imBd.descriptorPool, imBd.descriptorSetLayout, imBd.fontSImage);
	io.Fonts->SetTexID((ImTextureID)imBd.fontDescriptorSet);
}


void ImGui_ImplVulkan_SetMinImageCount(uint32_t min_image_count)
{
	VkEImgui_Backend* bd = VkEImgui_GetBackendData();

	IM_ASSERT(min_image_count >= 2);
	if (bd->minImageCount == min_image_count)
		return;

	IM_ASSERT(0); // FIXME-VIEWPORT: Unsupported. Need to recreate all swap chains!
	//ImGui_ImplVulkan_InitInfo* v = &bd->VulkanInitInfo;
	//VkResult err = vkDeviceWaitIdle(v->Device);
	//check_vk_result(err);
	//ImGui_ImplVulkanH_DestroyAllViewportsRenderBuffers(v->Device, v->Allocator);

	//bd->VulkanInitInfo.MinImageCount = min_image_count;
}


void VkEImgui_cleanupSwapChain(VkEImgui_Backend& imBd) {

	VkDevice device = imBd.engine->getBackEndData().device;
	for (size_t i = 0; i < imBd.frameBuffers.size(); i++) {
		vkDestroyFramebuffer(device, imBd.frameBuffers[i], nullptr);
	}
	vkFreeCommandBuffers(device, imBd.commandPool, static_cast<uint32_t>(imBd.commandBuffers.size()), imBd.commandBuffers.data());
	vkDestroyRenderPass(device, imBd.renderPass, nullptr);
}

void recreateImguiSwapChainObjects(VkEImgui_Backend& imBd, VkEImgui_DeviceObjectsInfo info) {
	VulkanEngine* vk = imBd.engine;
	VulkanBackEndData bd = vk->getBackEndData();
	VkE_SwapChain* sc = vk->getSwapChainDetails();

	ImGui_ImplVulkan_SetMinImageCount(sc->minImageCount);
	imBd.commandBuffers = vk->createCommandBuffers(imBd.commandPool, sc->imageCount);
	imBd.renderPass = vk->createRenderPass(sc->format, VK_SAMPLE_COUNT_1_BIT, info.firstPass, true,false, info.firstPass);
	imBd.frameBuffers = vk->createFramebuffers(imBd.renderPass, *sc);
}


void VkEImgui_cleanupBackEndObjects(VkEImgui_Backend& imBd) {
	VkDevice device = imBd.engine->getBackEndData().device;

	// First destroy objects in all viewports
	std::vector<VkEImgui_vertexBuffers>& vertexBuffers = imBd.mainViewport.vertexBuffers;
	// Resources to destroy when the program ends
	for (int i = 0; i < vertexBuffers.size(); i++) {
		if (vertexBuffers[i].vertex.buffer) { vkDestroyBuffer(device, vertexBuffers[i].vertex.buffer, nullptr); vertexBuffers[i].vertex.buffer = VK_NULL_HANDLE; }
		if (vertexBuffers[i].vertex.memory) { vkFreeMemory(device, vertexBuffers[i].vertex.memory, nullptr); vertexBuffers[i].vertex.memory = VK_NULL_HANDLE; }
		if (vertexBuffers[i].index.buffer)  { vkDestroyBuffer(device, vertexBuffers[i].index.buffer, nullptr); vertexBuffers[i].index.buffer = VK_NULL_HANDLE; }
		if (vertexBuffers[i].index.memory)  { vkFreeMemory(device, vertexBuffers[i].index.memory, nullptr); vertexBuffers[i].index.memory = VK_NULL_HANDLE; }
		vertexBuffers[i].vertex.size = 0;
		vertexBuffers[i].index.size = 0;
	}
	if (imBd.ShaderModuleVert) { vkDestroyShaderModule(device, imBd.ShaderModuleVert, nullptr); imBd.ShaderModuleVert = VK_NULL_HANDLE; }
	if (imBd.ShaderModuleFrag) { vkDestroyShaderModule(device, imBd.ShaderModuleFrag, nullptr); imBd.ShaderModuleFrag = VK_NULL_HANDLE; }
	if (imBd.descriptorPool) { vkDestroyDescriptorPool(device, imBd.descriptorPool, nullptr); imBd.descriptorPool = VK_NULL_HANDLE; }
	if (imBd.commandPool) { vkDestroyCommandPool(device, imBd.commandPool, nullptr); imBd.commandPool = VK_NULL_HANDLE; }
	if (imBd.descriptorSetLayout) { vkDestroyDescriptorSetLayout(device, imBd.descriptorSetLayout, nullptr); imBd.descriptorSetLayout = VK_NULL_HANDLE; }
	if (imBd.pipelineLayout) { vkDestroyPipelineLayout(device, imBd.pipelineLayout, nullptr); imBd.pipelineLayout = VK_NULL_HANDLE; }
	if (imBd.pipeline) { vkDestroyPipeline(device, imBd.pipeline, nullptr); imBd.pipeline = VK_NULL_HANDLE; }
	imBd.engine->cleanupSampledImage(imBd.fontSImage);
}

void VkEImgui_Shutdown()
{
	VkEImgui_Backend* bd = VkEImgui_GetBackendData();
	IM_ASSERT(bd != NULL && "No renderer backend to shutdown, or already shutdown?");
	ImGuiIO& io = ImGui::GetIO();


	// Manually delete main viewport render data in-case we haven't initialized for viewports
	ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	main_viewport->RendererUserData = NULL;

	// Clean up windows
	ImGui::DestroyPlatformWindows();

	io.BackendRendererName = NULL;
	io.BackendRendererUserData = NULL;
	//IM_DELETE(bd);
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

