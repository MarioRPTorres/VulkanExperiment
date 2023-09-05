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
#========================== Example generator ============================
if(${VKENGINE_BUILD_EXAMPLES})

CHECKVARIABLE(OPENCV_BIN_DIR)
CHECKVARIABLE(TINYOBJLOADER_SOURCE_DIR)

#=================== opencv 4.6.0 ====================

message( STATUS "CMake build type not recognized: ${CMAKE_DEBUG_POSTFIX}" )
if(NOT ((${CMAKE_BUILD_TYPE} STREQUAL "Debug") OR (${CMAKE_BUILD_TYPE} STREQUAL "Release")) )
  message( FATAL_ERROR "CMake build type not recognized: ${CMAKE_BUILD_TYPE}" )
endif()
 
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug") 
set(OPENCVDLL_POSTFIX "d")
else ()
set(OPENCVDLL_POSTFIX "")
endif()

# OpenCV_DIR could also be read from system environment variable.
set(OpenCV_DIR ${OPENCV_BIN_DIR})

find_package(OpenCV 4.6.0 REQUIRED COMPONENTS core imgproc imgcodecs)

set(OPENCV_DLL_DIR "${OpenCV_LIB_PATH}/../bin")

if(DEFINED OPENCV_STATIC AND NOT ${OPENCV_STATIC})
set(OPENCV_DLL_FILELIST "opencv_core460" "opencv_imgcodecs460" "opencv_imgproc460" )
else()
set(OPENCV_DLL_FILELIST "opencv_world460")
endif()

MESSAGE(STATUS "Opencv libraries: ${OpenCV_LIBS}")

MESSAGE(STATUS "Opencv Include Directories: ${OpenCV_INCLUDE_DIRS}")

#============================================================================

function(configureExampleTarget EXAMPLE_TARGET SCORE_SELECTION USE_IMGUI)

    # Set C++ Standard
    set_property(TARGET ${EXAMPLE_TARGET} PROPERTY CXX_STANDARD 11)

    MESSAGE(STATUS "Building example: ${EXAMPLE_TARGET}\nSCORE SELECTION ${SCORE_SELECTION}\nIMGUI ${USE_IMGUI}") 
    # Define score selection
    if(${SCORE_SELECTION})
        target_compile_definitions(${EXAMPLE_TARGET} PRIVATE PHYSICAL_DEVICE_SCORE_SELECTION)
    endif()
    # Define IMGUI implementation
    if(${USE_IMGUI})
        target_compile_definitions(${EXAMPLE_TARGET} PRIVATE IMGUI_EXT)
        target_link_libraries(${EXAMPLE_TARGET} PRIVATE ${IMGUI_TARGET} IMGUI)
    endif()

    target_compile_definitions(${EXAMPLE_TARGET} PRIVATE INCLUDE_GLM Vulkan_GLSLC_EXECUTABLE=${Vulkan_GLSLC_EXECUTABLE})
    # Include directories
    target_include_directories(${EXAMPLE_TARGET} PUBLIC "../include" )
    # Include loaded libraries 
    target_link_libraries(${EXAMPLE_TARGET} PRIVATE ${PROJECT_TARGET})
   
    #========================= OpenCV ====================
    target_include_directories(${EXAMPLE_TARGET} PUBLIC ${OpenCV_INCLUDE_DIRS} ) # not needed for opencv>=4.0

    target_link_libraries(${EXAMPLE_TARGET} PUBLIC ${OpenCV_LIBS})
    #===== tiny_object_loader 2.0.0 release candidate ====

    target_include_directories(${EXAMPLE_TARGET} PRIVATE  ${TINYOBJLOADER_SOURCE_DIR} ) 

    #==================== File dependencies ===============
    # Move the resource folder together with the executable after build
    ADD_CUSTOM_COMMAND(TARGET ${EXAMPLE_TARGET}
              POST_BUILD
              #COMMAND cmake_path  command_Mode command 
              COMMAND ${CMAKE_COMMAND} -E make_directory   $<TARGET_FILE_DIR:${EXAMPLE_TARGET}>/shaders 
              COMMAND ${CMAKE_COMMAND} -E make_directory   $<TARGET_FILE_DIR:${EXAMPLE_TARGET}>/textures 
              COMMAND ${CMAKE_COMMAND} -E make_directory   $<TARGET_FILE_DIR:${EXAMPLE_TARGET}>/models 
    )

    ## ------------
    # Get all shaders files added from subdirectory shaders
    FILE(GLOB shadersFiles "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*")
    message(STATUS "Resource files to copy: ${shadersFiles}")

    foreach(rFile ${shadersFiles})
    ADD_CUSTOM_COMMAND(TARGET ${EXAMPLE_TARGET}
              POST_BUILD
              #COMMAND cmake_path  command_Mode command 
              COMMAND ${CMAKE_COMMAND} -E copy ${rFile} $<TARGET_FILE_DIR:${EXAMPLE_TARGET}>/shaders
    )
    endforeach()

    ## ------------
    ## Copy all Opencv dll files
    foreach(dllFile ${OPENCV_DLL_FILELIST})
    ADD_CUSTOM_COMMAND(TARGET ${EXAMPLE_TARGET}
              POST_BUILD
              #COMMAND cmake_path  command_Mode command 
              COMMAND ${CMAKE_COMMAND} -E copy "${OPENCV_DLL_DIR}/${dllFile}${OPENCVDLL_POSTFIX}.dll" $<TARGET_FILE_DIR:${EXAMPLE_TARGET}>
    )
    endforeach()

    ## ------------
    # Get all texture files added from subdirectory textures
    FILE(GLOB textureFiles "${CMAKE_CURRENT_SOURCE_DIR}/textures/*")
    message(STATUS "Texture files to copy: ${textureFiles}")

    foreach(tFile ${textureFiles})
    ADD_CUSTOM_COMMAND(TARGET ${EXAMPLE_TARGET}
              POST_BUILD
              #COMMAND cmake_path  command_Mode command 
              COMMAND ${CMAKE_COMMAND} -E copy ${tFile} $<TARGET_FILE_DIR:${EXAMPLE_TARGET}>/textures
    )
    endforeach()

    ## ------------
    # Get all model files added from subdirectory models
    FILE(GLOB modelFiles "${CMAKE_CURRENT_SOURCE_DIR}/models/*")
    message(STATUS "Model files to copy: ${modelFiles}")

    foreach(mFile ${modelFiles})
    ADD_CUSTOM_COMMAND(TARGET ${EXAMPLE_TARGET}
              POST_BUILD
              #COMMAND cmake_path  command_Mode command 
              COMMAND ${CMAKE_COMMAND} -E copy ${mFile} $<TARGET_FILE_DIR:${EXAMPLE_TARGET}>/models
    )
    endforeach()

endfunction()

# Create the Vulkan Tutorial application target using the VulkanTutorial.cpp example 
add_executable( VulkanTutorial examples/VulkanTutorial.cpp )
CONFIGUREEXAMPLETARGET(VulkanTutorial TRUE TRUE)

# Vulkan Tutorial example without choosing a physical device based on score
add_executable( VulkanTutorial_noScoreSelection examples/VulkanTutorial.cpp )
CONFIGUREEXAMPLETARGET(VulkanTutorial_noScoreSelection FALSE TRUE)

# Vulkan Tutorial example without imgui
add_executable( VulkanTutorial_NoImgui examples/VulkanTutorial.cpp )
CONFIGUREEXAMPLETARGET(VulkanTutorial_NoImgui TRUE FALSE)

# Application for testing shaders from the ShaderToy website mainly
add_executable( ShaderToyLoad examples/ShaderToyLoad.cpp )
CONFIGUREEXAMPLETARGET(ShaderToyLoad TRUE FALSE)

# Application for drawing multiple points with united lines and cool moving points effects
add_executable( PointsDrawing examples/PointsDrawing.cpp )
CONFIGUREEXAMPLETARGET(PointsDrawing TRUE TRUE)

# Application to test out multiple windows for imgui and vulkan rendering
add_executable( MultipleWindows examples/MultipleWindows.cpp )
CONFIGUREEXAMPLETARGET(MultipleWindows TRUE FALSE)

# Application for different use of textures and texture coordinates
add_executable( PicturePeekaboo examples/PicturePeekaboo.cpp )
CONFIGUREEXAMPLETARGET(PicturePeekaboo TRUE FALSE)

# Application for testing out features used for DeepLeap application
add_executable( DeepLeap examples/DeepLeap.cpp )
CONFIGUREEXAMPLETARGET(DeepLeap TRUE TRUE)

endif()