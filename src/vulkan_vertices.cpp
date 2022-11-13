#include "vulkan_vertices.h"

BufferBundle createVertexBuffer(VulkanEngine* vk, std::vector<Vertex> vertices) {
	BufferBundle vertexBuffer;
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	// Create a host visible buffer
	BufferBundle stagingBuffer;
	vk->createBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer.buffer,
		stagingBuffer.memory);

	vk->mapBufferMemory(stagingBuffer.memory, vertices.data(), bufferSize);

	vk->createBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer.buffer,
		vertexBuffer.memory);
	vk->copyBuffer(stagingBuffer.buffer, vertexBuffer.buffer, bufferSize);

	vk->destroyBufferBundle(stagingBuffer);

	return vertexBuffer;
}