#include "vulkan_vertices.h"


namespace constants
{
	extern const std::array<VkVertexInputAttributeDescription, 4> PCTVertexAttributes{ {
		{
			0, // location
			0, // binding
			VK_FORMAT_R32G32B32_SFLOAT, // format
			offsetof(PCTVertex,pos) // offset
		},{
			1, // location
			0, // binding
			VK_FORMAT_R32G32B32_SFLOAT, // format
			offsetof(PCTVertex,color) // offset
		},{
			2, // location
			0, // binding
			VK_FORMAT_R32G32_SFLOAT, // format
			offsetof(PCTVertex,texCoord) // offset
		},{
			3, // location
			0, // binding
			VK_FORMAT_R32_SINT, // format
			offsetof(PCTVertex,texId) // offset
		},
}};
	extern const std::array<VkVertexInputAttributeDescription, 2> CP2VertexAttributes{{
		{
			0, // location
			0, // binding
			VK_FORMAT_R32G32B32_SFLOAT, // format
			offsetof(CP2Vertex,color) // offset
		},{
			1, // location
			0, // binding
			VK_FORMAT_R32G32_SFLOAT, // format
			offsetof(CP2Vertex,pos) // offset
		}
} };
	extern const std::array<VkVertexInputAttributeDescription, 1> P2VertexAttributes{ {
	{
		0, // location
		0, // binding
		VK_FORMAT_R32G32_SFLOAT, // format
		offsetof(P2Vertex,pos) // offset
	}
} };
	extern const std::array<VkVertexInputAttributeDescription, 1> P3VertexAttributes{ {
	{
		0, // location
		0, // binding
		VK_FORMAT_R32G32B32_SFLOAT, // format
		offsetof(P3Vertex,pos) // offset
	}
} };
}
