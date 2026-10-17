
#include <glad/glad.h> /* glad gives you the OpenGL functions */
#include <GLFW/glfw3.h>/*GLFW handles the window,keyboard, and mouse since OpenGL itself can't open a window. */
#include <glm/glm.hpp> /* glm is a math library for vectors and matrices */
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

/* 
R"(...)" stores the shader as text in C++, then gets sent to the GPU and compiled when the 
when the program runs.
#version 330 core means "this is written for OpenGL3.3"
layout(location = 0) in vec3 aPos; is an input: the corner's position (x,y,z). Its slot 0
filled from the vertices array.
layout(location = 1) in vec3 aColour; is an input: the corner's colour (r,g,b). Its slot 1.
out vec3 vColour; is an output that sends the colour to the next stage (the fragment shader).
uniform mat4 uMVP; is a 4x4 matrix sent from C++ each frame. "Uniform" means it's the same for every corner. 
Holds rotation, camera, and perspective combined.
void main() is where the shader starts running, once per corner.
vColor = aColor; passes the color through unchanged.
gl_Position = uMVP * vec4(aPos, 1.0); is the key line it multiplies the corner's position
by the matrix to get its screen position. 
vec4(aPos, 1.0) adds a 4th number (1) so it fits the 4x4 matrix. 
*/
const char* vsSrc = R"(               
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
out vec3 vColor;
uniform mat4 uMVP;
void main() {
    vColor = aColor;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

const char* fsSrc = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type); 
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        std::cerr << "Shader error:\n" << log << "\n";
    }
    return s;
}

int main() {
    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return -1; }

    // macOS supports core profile 3.3+ only with these hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Rotating Cube", nullptr, nullptr);
    if (!window) { std::cerr << "Window creation failed\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD load failed\n"; return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Shader program
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // 8 corners: position (xyz) + color (rgb)
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  1, 0, 0,
         0.5f, -0.5f, -0.5f,  0, 1, 0,
         0.5f,  0.5f, -0.5f,  0, 0, 1,
        -0.5f,  0.5f, -0.5f,  1, 1, 0,
        -0.5f, -0.5f,  0.5f,  1, 0, 1,
         0.5f, -0.5f,  0.5f,  0, 1, 1,
         0.5f,  0.5f,  0.5f,  1, 1, 1,
        -0.5f,  0.5f,  0.5f,  0.2f, 0.2f, 0.2f,
    };
    unsigned int indices[] = {
        0,1,2, 2,3,0,   // back
        4,5,6, 6,7,4,   // front
        0,3,7, 7,4,0,   // left
        1,5,6, 6,2,1,   // right
        0,1,5, 5,4,0,   // bottom
        3,2,6, 6,7,3,   // top
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLint mvpLoc = glGetUniformLocation(prog, "uMVP");

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Framebuffer size (not window size) matters on Retina displays
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float t = (float)glfwGetTime();
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), t, glm::vec3(0.5f, 1.0f, 0.0f));
        glm::mat4 view  = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
        glm::mat4 proj  = glm::perspective(glm::radians(45.0f), (float)w / (float)(h ? h : 1), 0.1f, 100.0f);
        glm::mat4 mvp   = proj * view * model;

        glUseProgram(prog);
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(prog);
    glfwTerminate();
    return 0;
}
