cmake_minimum_required(VERSION 3.10)
project(MeshViewer)

set(CMAKE_CXX_STANDARD 17)
set(OpenGL_GL_PREFERENCE LEGACY)

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})

# glm
find_package(glm REQUIRED)

# GLUT
find_package(GLUT REQUIRED)

# Hard-coded paths for Homebrew (M1 Mac)
set(GLEW_INCLUDE_DIR "/opt/homebrew/include")
set(GLEW_LIBRARY "/opt/homebrew/lib/libGLEW.dylib")

include_directories(
    ${GLEW_INCLUDE_DIR}
    ${GLUT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCE_FILES 
    main.cpp
    myHalfedge.cpp
    myVector3D.cpp
    myPoint3D.cpp
    myFace.cpp
    myMesh.cpp
    myVertex.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARY}
    ${GLUT_LIBRARIES}
    glm::glm
)