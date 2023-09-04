#========================== Example generator ============================
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
    FILE(GLOB shadersFiles "${CMAKE_CURRENT_SOURCE_DIR}/../shaders/*")
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
    FILE(GLOB textureFiles "${CMAKE_CURRENT_SOURCE_DIR}/../textures/*")
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
    FILE(GLOB modelFiles "${CMAKE_CURRENT_SOURCE_DIR}/../models/*")
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
add_executable( VulkanTutorial VulkanTutorial.cpp )
CONFIGUREEXAMPLETARGET(VulkanTutorial TRUE TRUE)

# Vulkan Tutorial example without choosing a physical device based on score
add_executable( VulkanTutorial_noScoreSelection VulkanTutorial.cpp )
CONFIGUREEXAMPLETARGET(VulkanTutorial_noScoreSelection FALSE TRUE)

# Vulkan Tutorial example without imgui
add_executable( VulkanTutorial_NoImgui VulkanTutorial.cpp )
CONFIGUREEXAMPLETARGET(VulkanTutorial_NoImgui TRUE FALSE)

# Application for testing shaders from the ShaderToy website mainly
add_executable( ShaderToyLoad ShaderToyLoad.cpp )
CONFIGUREEXAMPLETARGET(ShaderToyLoad TRUE FALSE)

# Application for drawing multiple points with united lines and cool moving points effects
add_executable( PointsDrawing PointsDrawing.cpp )
CONFIGUREEXAMPLETARGET(PointsDrawing TRUE TRUE)

# Application to test out multiple windows for imgui and vulkan rendering
add_executable( MultipleWindows MultipleWindows.cpp )
CONFIGUREEXAMPLETARGET(MultipleWindows TRUE FALSE)

# Application for different use of textures and texture coordinates
add_executable( PicturePeekaboo PicturePeekaboo.cpp )
CONFIGUREEXAMPLETARGET(PicturePeekaboo TRUE FALSE)

# Application for testing out features used for DeepLeap application
add_executable( DeepLeap DeepLeap.cpp )
CONFIGUREEXAMPLETARGET(DeepLeap TRUE TRUE)