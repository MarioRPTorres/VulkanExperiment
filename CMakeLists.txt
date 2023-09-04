cmake_minimum_required(VERSION 3.22)
project(VulkanExperiment VERSION 1.0.0 LANGUAGES C CXX)

# Use cmake --log-level=debug to get debug output.
set( CMAKE_MESSAGE_LOG_LEVEL "STATUS" )

set(VKENGINE_ROOT_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(VKENGINE_ROOT_PROJECT ON)
endif ()

option(VKENGINE_BUILD_EXAMPLES "Build examples for vulkan engine" ${VKENGINE_ROOT_PROJECT})
option(VKENGINE_INSTALL "Generate install target for nfd" ${VKENGINE_ROOT_PROJECT})
option(VKENGINE_IMGUI_BUILD "Build the static library for Imgui" ON)

message(STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
# Set Target Name
set(PROJECT_TARGET VulkanEngine)
set(IMGUI_TARGET VulkanEngine_Imgui)

function(checkVariable VARIABLE_NAME)
    if(NOT DEFINED ${VARIABLE_NAME})
        message( FATAL_ERROR "${VARIABLE_NAME} not defined" )
    else()
        message( STATUS "${VARIABLE_NAME}: ${${VARIABLE_NAME}}")
    endif(NOT DEFINED ${VARIABLE_NAME})
endfunction()

CHECKVARIABLE(GLFW_SOURCE_DIR)
CHECKVARIABLE(GLM_SOURCE_DIR)
 
#======================================  Libraries =================================
#=================== GLM ===================
# GLM library 0.9.9.8
if(NOT TARGET glm::glm)
    set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    add_subdirectory(${GLM_SOURCE_DIR} "./build/glm" EXCLUDE_FROM_ALL)
endif()
#=================== GLFW ===================
# GLFW library 3.3.7
# Turn off extra GLFW build features
if (NOT TARGET glfw)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE) 

    add_subdirectory(${GLFW_SOURCE_DIR} "./build/glfw" EXCLUDE_FROM_ALL)
endif()
#=================== Vulkan ===================
# Needs system varialble for the Vulkan path VK_SDK_PATH defined (or VULKAN_SDK)
find_package(Vulkan REQUIRED)
if (NOT Vulkan_FOUND)
    message( FATAL_ERROR "Vulkan package not found. Try setting VK_SDK_PATH or VULKAN_SDK system variable" )
    return()
endif()
message( STATUS "Vulkan glslc executable: ${Vulkan_GLSLC_EXECUTABLE}" )
#=================== IMGUI ===================
if(${VKENGINE_IMGUI_BUILD} AND NOT TARGET IMGUI)
    CHECKVARIABLE(IMGUI_SOURCE_DIR)
    # Dear Imgui Library Docking WIP 1.88
    add_library(IMGUI STATIC)

    target_sources( IMGUI
                    PRIVATE
                        ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
                        ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
                        ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
                        ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
                        ${IMGUI_SOURCE_DIR}/imgui.cpp

                    PRIVATE
                        ${IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
                    )

    target_include_directories( IMGUI
                                PUBLIC ${IMGUI_SOURCE_DIR}
                                PUBLIC ${IMGUI_SOURCE_DIR}/backends
                                )

    target_link_libraries(IMGUI PRIVATE glfw "Vulkan::Vulkan")
endif()

#======================================  Build VulkanEngine Target =================================

# Create a static library with the modules
add_library(${PROJECT_TARGET} STATIC  
"src/vulkan_engine.cpp" 
"src/glfwInteraction.cpp" 
"src/importResources.cpp" 
"src/vulkan_vertices.cpp" 
"src/vulkan_descriptors.cpp")

#Define the C++ Standard
set_property(TARGET ${PROJECT_TARGET} PROPERTY CXX_STANDARD 11)

# This project specific include files
target_include_directories(${PROJECT_TARGET} PUBLIC "./include" )

set_target_properties(${PROJECT_TARGET} PROPERTIES
  # "current version" in semantic format in Mach-O binary file
  VERSION 1.0.0-alpha
  PUBLIC_HEADER "include/vulkan_engine.h;include/vulkan_descriptors.h;include/vulkan_vertices.h"
)

target_link_libraries(${PROJECT_TARGET} PUBLIC glm::glm glfw "Vulkan::Vulkan")

#======================================  Build VulkanEngine_Imgui Target =================================
if(${VKENGINE_IMGUI_BUILD})
    
    add_library(${IMGUI_TARGET} STATIC  
        "src/vulkan_imgui.cpp")
   
    set_property(TARGET ${IMGUI_TARGET} PROPERTY CXX_STANDARD 11)
    
    target_include_directories(${IMGUI_TARGET} PUBLIC "./include" )
    
    set_target_properties(${IMGUI_TARGET} PROPERTIES
      # "current version" in semantic format in Mach-O binary file
      VERSION 1.0.0-alpha
      PUBLIC_HEADER "include/vulkan_imgui.h"
    )
    
    target_link_libraries(${IMGUI_TARGET} PRIVATE IMGUI ${PROJECT_TARGET})
endif()
#=================== Final Linking ======================

if(${VKENGINE_INSTALL})
    message(STATUS "CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR}")
    install(TARGETS ${PROJECT_TARGET} 
            PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/${CMAKE_INSTALL_INCLUDEDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/lib"
            CONFIGURATIONS ${CMAKE_BUILD_TYPE}
    )
    if (${VKENGINE_IMGUI_BUILD})
        install(TARGETS ${IMGUI_TARGET} 
            PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/${CMAKE_INSTALL_INCLUDEDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/lib"
            CONFIGURATIONS ${CMAKE_BUILD_TYPE}
        )
    endif()
    install(FILES "resources/CMakeLists.txt" DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}" )
endif()
#======================= Examples =========================
if(${VKENGINE_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif()