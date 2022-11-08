cmake_minimum_required(VERSION 3.22)

# Set Target Name
set(PROJECT_TARGET VulkanExperiment)

project(${PROJECT_TARGET} VERSION 1.0.0 LANGUAGES C CXX)

# Use cmake --log-level=debug to get debug output.
set( CMAKE_MESSAGE_LOG_LEVEL "STATUS" )

# This project specific include files
include_directories(./src )

# Create the project target
add_executable(${PROJECT_TARGET} 
	src/main.cpp
 "src/vulkan_engine.cpp" "src/glfwInteraction.cpp" "src/importResources.cpp" "src/vulkan_imgui.cpp")

 # Define macros
target_compile_definitions(${PROJECT_TARGET} PRIVATE IMGUI_EXT)

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

target_link_libraries(IMGUI PUBLIC glfw "Vulkan::Vulkan")

target_link_libraries(${PROJECT_TARGET} PRIVATE IMGUI)

#=================== opencv 4.6.0 ===================

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

include_directories( ${OpenCV_INCLUDE_DIRS}) # not needed for opencv>=4.0

target_link_libraries(${PROJECT_TARGET} PUBLIC ${OpenCV_LIBS})

set(OPENCV_DLL_DIR "${OpenCV_LIB_PATH}/../bin")

set(OPENCV_DLL_FILELIST "opencv_core460" "opencv_imgcodecs460" "opencv_imgproc460" )

MESSAGE(STATUS "Opencv libraries: ${OpenCV_LIBS}")

MESSAGE(STATUS "Opencv Include Directories: ${OpenCV_INCLUDE_DIRS}")


#=================== tiny_object_loader 2.0.0 release candidate ===================

include_directories("./libs/tinyobjloader-2.0.0-rc")

#==============================================

#Define the C++ Standard
set_property(TARGET ${PROJECT_TARGET} PROPERTY CXX_STANDARD 11)



# Move the resource folder together with the executable after build
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_TARGET}
          POST_BUILD
          #COMMAND cmake_path  command_Mode command 
          COMMAND ${CMAKE_COMMAND} -E make_directory   $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources 
          COMMAND ${CMAKE_COMMAND} -E make_directory   $<TARGET_FILE_DIR:${PROJECT_NAME}>/textures 
          COMMAND ${CMAKE_COMMAND} -E make_directory   $<TARGET_FILE_DIR:${PROJECT_NAME}>/models 
)

# Get all resource files added from subdirectory resources
FILE(GLOB resourcesFiles "${CMAKE_CURRENT_SOURCE_DIR}/resources/*")
message(STATUS "Resource files to copy: ${resourcesFiles}")

foreach(rFile ${resourcesFiles})
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_TARGET}
          POST_BUILD
          #COMMAND cmake_path  command_Mode command 
          COMMAND ${CMAKE_COMMAND} -E copy ${rFile} $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)
endforeach()

foreach(dllFile ${OPENCV_DLL_FILELIST})
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_TARGET}
          POST_BUILD
          #COMMAND cmake_path  command_Mode command 
          COMMAND ${CMAKE_COMMAND} -E copy "${OPENCV_DLL_DIR}/${dllFile}${OPENCVDLL_POSTFIX}.dll" $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
endforeach()

# Get all texture files added from subdirectory resources
FILE(GLOB textureFiles "${CMAKE_CURRENT_SOURCE_DIR}/textures/*")
message(STATUS "Texture files to copy: ${textureFiles}")

foreach(tFile ${textureFiles})
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_TARGET}
          POST_BUILD
          #COMMAND cmake_path  command_Mode command 
          COMMAND ${CMAKE_COMMAND} -E copy ${tFile} $<TARGET_FILE_DIR:${PROJECT_NAME}>/textures
)
endforeach()

# Get all model files added from subdirectory resources
FILE(GLOB modelFiles "${CMAKE_CURRENT_SOURCE_DIR}/models/*")
message(STATUS "Model files to copy: ${modelFiles}")

foreach(mFile ${modelFiles})
ADD_CUSTOM_COMMAND(TARGET ${PROJECT_TARGET}
          POST_BUILD
          #COMMAND cmake_path  command_Mode command 
          COMMAND ${CMAKE_COMMAND} -E copy ${mFile} $<TARGET_FILE_DIR:${PROJECT_NAME}>/models
)
endforeach()