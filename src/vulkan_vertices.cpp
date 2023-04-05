#include "vulkan_vertices.h"


namespace constants
{
	// actual global variables
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
}
