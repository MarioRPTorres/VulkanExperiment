cmake_minimum_required(VERSION 3.22)

add_library(VulkanEngine STATIC IMPORTED GLOBAL)

set_property(TARGET VulkanEngine PROPERTY 
			IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/VulkanEngine.lib")

target_include_directories(VulkanEngine INTERFACE  "${CMAKE_CURRENT_SOURCE_DIR}/include")


add_library(VulkanEngine_Imgui STATIC IMPORTED GLOBAL)

set_property(TARGET VulkanEngine_Imgui PROPERTY 
			IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/VulkanEngine_Imgui.lib")

target_include_directories(VulkanEngine_Imgui INTERFACE  "${CMAKE_CURRENT_SOURCE_DIR}/include")