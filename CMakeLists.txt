cmake_minimum_required(VERSION 3.10)
project(OpenGLCube)
set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

include_directories(libs/glad/include)

add_executable(OpenGLCube
    src/main.cpp
    libs/glad/src/glad.c
)

target_link_libraries(OpenGLCube glfw glm::glm)

if(APPLE)
    target_link_libraries(OpenGLCube "-framework OpenGL")
    add_compile_definitions(GL_SILENCE_DEPRECATION)
endif()
