cmake_minimum_required(VERSION 3.22)

add_library(VulkanEngine STATIC IMPORTED GLOBAL)

set_property(TARGET VulkanEngine PROPERTY 
			IMPORTED_LOCATION "./lib/VulkanEngine.lib")


add_library(VulkanEngine_Imgui STATIC IMPORTED GLOBAL)

set_property(TARGET VulkanEngine_Imgui PROPERTY 
			IMPORTED_LOCATION "./lib/VulkanEngine_Imgui.lib")

set(OpenCV_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include" GLOBAL)