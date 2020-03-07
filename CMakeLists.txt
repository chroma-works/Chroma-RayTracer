
cmake_minimum_required(VERSION 2.6)

project(chroma-ray-tracer)

# Set the output folder where your program will be created
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# The following folder will be included
include_directories(src)


#include glfw
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
add_subdirectory(src/thirdparty/glfw-3.3)

#include spdlog
add_subdirectory(src/thirdparty/spdlog-1.3.1)

set (thirdparty_sources
	src/thirdparty/tinyxml2/tinyxml2.h
	src/thirdparty/tinyxml2/tinyxml2.cpp
    src/thirdparty/glad/include/glad/glad.h
    src/thirdparty/glad/include/KHR/khrplatform.h
    src/thirdparty/glad/src/glad.c
    src/thirdparty/imgui/imconfig.h
    src/thirdparty/imgui/imgui.cpp
    src/thirdparty/imgui/imgui.h
	src/thirdparty/imgui/imgui_impl_opengl3.h
	src/thirdparty/imgui/imgui_impl_opengl3.cpp
	src/thirdparty/imgui/imgui_impl_glfw.h
	src/thirdparty/imgui/imgui_impl_glfw.cpp
    src/thirdparty/imgui/imgui_demo.cpp
    src/thirdparty/imgui/imgui_draw.cpp
    src/thirdparty/imgui/imgui_internal.h
    src/thirdparty/imgui/imgui_widgets.cpp
	src/thirdparty/stb_image/stb_image.h
	src/thirdparty/stb_image/stb_image_write.h
	src/thirdparty/OBJ_loader/OBJ_Loader.h
	src/thirdparty/hapPLy/happly.h
)

source_group ("thirdparty\\" FILES
    ${thirdparty_sources}
)

set (main_sources
	src/ray-tracer/main/Camera.h
	src/ray-tracer/main/Camera.cpp
	src/ray-tracer/main/EntryPoint.cpp
	src/ray-tracer/main/Shape.h
	src/ray-tracer/main/Geometry.h
	src/ray-tracer/main/Geometry.cpp
	src/ray-tracer/main/Material.h
	src/ray-tracer/main/Light.h
	src/ray-tracer/main/Image.h
	src/ray-tracer/main/Image.cpp
	src/ray-tracer/main/Ray.h
	src/ray-tracer/main/RayTracer.h
	src/ray-tracer/main/RayTracer.cpp
	src/ray-tracer/main/Scene.h
	src/ray-tracer/main/Scene.cpp
	src/ray-tracer/main/SceneObject.h
	src/ray-tracer/main/SceneObject.cpp
	src/ray-tracer/main/Window.h
	src/ray-tracer/main/Window.cpp
)
source_group ("main\\" FILES
    ${main_sources}
)

set (acc_sources
	src/ray-tracer/accelerationStructures/AccelerationStructure.h
	src/ray-tracer/accelerationStructures/BVH.h
	src/ray-tracer/accelerationStructures/BVH.cpp
	src/ray-tracer/accelerationStructures/Memory.h
	src/ray-tracer/accelerationStructures/Memory.cpp
)
source_group ("accelerationStructures\\" FILES
    ${acc_sources}
)

set (editor_sources
	src/ray-tracer/editor/AssetImporter.h
	src/ray-tracer/editor/AssetImporter.cpp
	src/ray-tracer/editor/Buffer.h
	src/ray-tracer/editor/Editor.h
	src/ray-tracer/editor/Editor.cpp
	src/ray-tracer/editor/Logger.h
	src/ray-tracer/editor/Logger.cpp
	src/ray-tracer/editor/Settings.h
	src/ray-tracer/editor/Shader.h
	src/ray-tracer/editor/Shader.cpp
	src/ray-tracer/editor/Texture.h
	src/ray-tracer/editor/Texture.cpp
)
source_group ("editor\\" FILES
    ${editor_sources}
)

set (opengl_sources
    src/ray-tracer/openGL/openGLBuffer.h
    src/ray-tracer/openGL/openGLBuffer.cpp
    src/ray-tracer/openGL/openGLVertexArrayObject.h
    src/ray-tracer/openGL/openGLVertexArrayObject.cpp
)

source_group ("opengl\\" FILES
    ${opengl_sources}
)

add_executable (chroma-ray-tracer
	${editor_sources}
    ${main_sources}
	${acc_sources}
	${opengl_sources}
    ${thirdparty_sources}
)

# Find and link to OpenGL
include(FindOpenGL)
target_link_libraries(chroma-ray-tracer ${OPENGL_gl_LIBRARY})

# Link to glfw
target_link_libraries(chroma-ray-tracer glfw)

#Link spdlog
target_link_libraries(chroma-ray-tracer spdlog)

#Windows
add_definitions(-DNOC_FILE_DIALOG_IMPLEMENTATION)
add_definitions(-DNOC_FILE_DIALOG_WIN32)


set_target_properties(
   chroma-ray-tracer
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

set_target_properties(
    chroma-ray-tracer
    PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY  "${CMAKE_SOURCE_DIR}/bin/$(Configuration)")

# Create logs directory
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/logs")