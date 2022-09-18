cmake_minimum_required(VERSION 3.22)

# Set Target Name
set(PROJECT_TARGET VulkanExperiment)

project(${PROJECT_TARGET} VERSION 1.0.0 LANGUAGES C CXX)

# Use cmake --log-level=debug to get debug output.
set( CMAKE_MESSAGE_LOG_LEVEL "STATUS" )

# This project specific include files
include_directories(./include )

# Create the project target
add_executable(${PROJECT_TARGET} 
	src/main.cpp
)

# Add libs
# add_subdirectory("libs")
#=================== GLM ===================
# GLM library 0.9.9.8
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS Off CACHE BOOL "" FORCE)

add_subdirectory(".\\libs\\glm-0.9.9.8")

target_link_libraries(${PROJECT_TARGET} PRIVATE glm::glm )
#=================== GLFW ===================
# GLFW library 3.3.7
# Turn off extra GLFW build features
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(".\\libs\\glfw-3.3.7")

target_link_libraries(${PROJECT_TARGET} PRIVATE glfw )

#=================== Vulkan ===================
find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_TARGET} PRIVATE "Vulkan::Vulkan")

#==============================================

#Define the C++ Standard
set_property(TARGET ${PROJECT_TARGET} PROPERTY CXX_STANDARD 11)

# Get all dll files added from subdirectory "libs"
FILE(GLOB resourcesFiles "${CMAKE_CURRENT_SOURCE_DIR}/resources/*")
message(STATUS "Resource files to copy: ${resourcesFiles}")

# Move the resource folder together with the executable after build
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_TARGET}
          POST_BUILD
          #COMMAND cmake_path  command_Mode command 
          COMMAND ${CMAKE_COMMAND} -E make_directory   $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources 
)

foreach(rFile ${resourcesFiles})
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_TARGET}
          POST_BUILD
          #COMMAND cmake_path  command_Mode command 
          COMMAND ${CMAKE_COMMAND} -E copy ${rFile} $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)
endforeach()