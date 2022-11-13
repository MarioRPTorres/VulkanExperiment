cmake_minimum_required(VERSION 3.22)

# Build Settings 
set(BUILD_MAIN_EXAMPLE TRUE)
set(BUILD_EXTRA_EXAMPLES FALSE)
 
# Set Target Name
set(PROJECT_TARGET VulkanEngine)
set(IMGUI_TARGET VulkanEngine_Imgui)

set(EXAMPLE_MAIN_TARGET VulkanTutorial)
set(EXAMPLE_NO_IMGUI VulkanTutorial_NoImgui)
set(EXAMPLE_NO_SCORE VulkanTutorial_noScoreSelection)

project(VulkanExperiment VERSION 1.0.0 LANGUAGES C CXX)

# Use cmake --log-level=debug to get debug output.
set( CMAKE_MESSAGE_LOG_LEVEL "STATUS" )



# Create a static library with the modules
add_library(${PROJECT_TARGET} STATIC  "src/vulkan_engine.cpp" "src/glfwInteraction.cpp" "src/importResources.cpp" "src/vulkan_vertices.cpp")
add_library(${IMGUI_TARGET} STATIC  "src/vulkan_imgui.cpp")

#Define the C++ Standard
set_property(TARGET ${PROJECT_TARGET} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${IMGUI_TARGET} PROPERTY CXX_STANDARD 11)

# This project specific include files
target_include_directories(${PROJECT_TARGET} PUBLIC "./include" )
target_include_directories(${IMGUI_TARGET} PUBLIC "./include" )

set_target_properties(${PROJECT_TARGET} PROPERTIES
  # "current version" in semantic format in Mach-O binary file
  VERSION 1.0.0-alpha
  PUBLIC_HEADER "include/vulkan_engine.h"
)
set_target_properties(${IMGUI_TARGET} PROPERTIES
  # "current version" in semantic format in Mach-O binary file
  VERSION 1.0.0-alpha
  PUBLIC_HEADER "include/vulkan_imgui.h"
)
#======================================  Libraries =================================
# Add libs
# add_subdirectory("libs")

#=================== GLM ===================
# GLM library 0.9.9.8
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(".\\libs\\glm-0.9.9.8")

#=================== GLFW ===================
# GLFW library 3.3.7
# Turn off extra GLFW build features
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(".\\libs\\glfw-3.3.7")

#=================== Vulkan ===================
# Needs system varialble for the Vulkan path VK_SDK_PATH defined (or VULKAN_SDK)
find_package(Vulkan REQUIRED)
if (NOT Vulkan_FOUND)
    message( FATAL_ERROR "Vulkan package not found. Try setting VK_SDK_PATH or VULKAN_SDK system variable" )
    return()
endif()
message( STATUS "Vulkan glslc executable: ${Vulkan_GLSLC_EXECUTABLE}" )
#=================== IMGUI ===================
# Dear Imgui Library Docking WIP 1.88
set(IMGUI_DIR ".\\libs\\imgui")
add_library(IMGUI STATIC)

target_sources( IMGUI
                PRIVATE
                    ${IMGUI_DIR}/imgui_demo.cpp
                    ${IMGUI_DIR}/imgui_draw.cpp
                    ${IMGUI_DIR}/imgui_tables.cpp
                    ${IMGUI_DIR}/imgui_widgets.cpp
                    ${IMGUI_DIR}/imgui.cpp

                PRIVATE
                    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
                )

target_include_directories( IMGUI
                            PUBLIC ${IMGUI_DIR}
                            PUBLIC ${IMGUI_DIR}/backends
                            )

target_link_libraries(IMGUI PRIVATE glfw "Vulkan::Vulkan")

#=================== Final Linking ======================

target_link_libraries(${PROJECT_TARGET} PUBLIC glm::glm glfw "Vulkan::Vulkan")
target_link_libraries(${IMGUI_TARGET} PRIVATE IMGUI ${PROJECT_TARGET})

install(TARGETS ${PROJECT_TARGET} ${IMGUI_TARGET} 
        PUBLIC_HEADER
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(FILES "resources/CMakeLists.txt" DESTINATION ${CMAKE_INSTALL_PREFIX} )

#======================= Examples =========================

#=================== Example Libraries ===============
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
set(OpenCV_DIR "C:\\.local\\opencv-4.6.0\\${CMAKE_BUILD_TYPE}")

find_package(OpenCV 4.6.0 REQUIRED COMPONENTS core imgproc imgcodecs)

set(OPENCV_DLL_DIR "${OpenCV_LIB_PATH}/../bin")

set(OPENCV_DLL_FILELIST "opencv_core460" "opencv_imgcodecs460" "opencv_imgproc460" )

MESSAGE(STATUS "Opencv libraries: ${OpenCV_LIBS}")

MESSAGE(STATUS "Opencv Include Directories: ${OpenCV_INCLUDE_DIRS}")

#========================== Example generator ============================

function(configureExampleTarget EXAMPLE_TARGET SCORE_SELECTION USE_IMGUI)

    # Set C++ Standard
    set_property(TARGET ${EXAMPLE_TARGET} PROPERTY CXX_STANDARD 11)

    MESSAGE(STATUS "SCORE SELECTION ${SCORE_SELECTION} IMGUI ${USE_IMGUI}") 
    # Define score selection
    if(${SCORE_SELECTION})
        target_compile_definitions(${EXAMPLE_TARGET} PRIVATE PHYSICAL_DEVICE_SCORE_SELECTION)
    endif()
    # Define IMGUI implementation
    if(${USE_IMGUI})
        target_compile_definitions(${EXAMPLE_TARGET} PRIVATE IMGUI_EXT)
        target_link_libraries(${EXAMPLE_TARGET} PRIVATE ${IMGUI_TARGET} IMGUI)
    endif()

    target_compile_definitions(${EXAMPLE_TARGET} PRIVATE Vulkan_GLSLC_EXECUTABLE=${Vulkan_GLSLC_EXECUTABLE})
    # Include directories
    target_include_directories(${EXAMPLE_TARGET} PUBLIC "./include" )
    # Include loaded libraries 
    target_link_libraries(${EXAMPLE_TARGET} PRIVATE ${PROJECT_TARGET})
   
    #========================= OpenCV ====================
    target_include_directories(${EXAMPLE_TARGET} PUBLIC ${OpenCV_INCLUDE_DIRS} ) # not needed for opencv>=4.0

    target_link_libraries(${EXAMPLE_TARGET} PUBLIC ${OpenCV_LIBS})
    #===== tiny_object_loader 2.0.0 release candidate ====

    target_include_directories(${EXAMPLE_TARGET} PRIVATE  "./libs/tinyobjloader-2.0.0-rc" ) 

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

if (BUILD_MAIN_EXAMPLE)
    # Create the Vulkan Tutorial application target using the main.cpp example 
    add_executable( ${EXAMPLE_MAIN_TARGET} 
	    example/VulkanTutorial.cpp
    )
    CONFIGUREEXAMPLETARGET(${EXAMPLE_MAIN_TARGET} TRUE TRUE)
endif()

if (BUILD_EXTRA_EXAMPLES)
    # Vulkan Tutorial example without choosing a physical device based on score
    add_executable( ${EXAMPLE_NO_SCORE} 
	    example/VulkanTutorial.cpp
    )
    CONFIGUREEXAMPLETARGET(${EXAMPLE_NO_SCORE} FALSE TRUE)

    # Vulkan Tutorial example without imgui
    add_executable( ${EXAMPLE_NO_IMGUI} 
	    example/VulkanTutorial.cpp
    )
    CONFIGUREEXAMPLETARGET(${EXAMPLE_NO_IMGUI} TRUE FALSE)
endif()