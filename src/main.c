#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>
#include <cglm/cglm.h>

const char* vertexShaderSource =
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main() {\n"
    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\n";

const char* fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
    "}\n";

// Global variables
float cameraSpeed = 0.5f;
float cameraYaw = -90.0f;
float cameraPitch = 0.0f;
float cameraSensitivity = 0.05f;
float lastX = 400, lastY = 300;
bool firstMouse = true;

vec3 cameraPos = {0.0f, 0.0f, 5.0f};
vec3 cameraFront = {0.0f, 0.0f, -1.0f};
vec3 cameraLeft = {1.0f, 0.0f, 0.0f};
vec3 cameraUp = {0.0f, 1.0f, 0.0f};

float lastFrame = 0.0f;
float deltaTime = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    float speed = deltaTime * 2.5f; // Movement speed

    vec3 right;
    glm_vec3_cross(cameraFront, cameraUp, right);
    glm_vec3_normalize(right);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        vec3 velocity;
        glm_vec3_scale(cameraFront, speed, velocity);
        glm_vec3_add(cameraPos, velocity, cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        vec3 velocity;
        glm_vec3_scale(cameraFront, speed, velocity);
        glm_vec3_sub(cameraPos, velocity, cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        vec3 velocity;
        glm_vec3_scale(right, speed, velocity);
        glm_vec3_sub(cameraPos, velocity, cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        vec3 velocity;
        glm_vec3_scale(right, speed, velocity);
        glm_vec3_add(cameraPos, velocity, cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        vec3 velocity;
        glm_vec3_scale(cameraUp, speed, velocity);
        glm_vec3_add(cameraPos, velocity, cameraPos);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        vec3 velocity;
        glm_vec3_scale(cameraUp, speed, velocity);
        glm_vec3_sub(cameraPos, velocity, cameraPos);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Inverted Y-axis
    lastX = xpos;
    lastY = ypos;

    xoffset *= cameraSensitivity;
    yoffset *= cameraSensitivity;

    cameraYaw += xoffset;
    cameraPitch += yoffset;

    if (cameraPitch > 90.0f)
        cameraPitch = 90.0f;
    if (cameraPitch < -90.0f)
        cameraPitch = -90.0f;

    vec3 front;
    front[0] = cos(glm_rad(cameraYaw)) * cos(glm_rad(cameraPitch));
    front[1] = sin(glm_rad(cameraPitch));
    front[2] = sin(glm_rad(cameraYaw)) * cos(glm_rad(cameraPitch));
    glm_vec3_normalize(front);
    glm_vec3_copy(front, cameraFront);
}

int compileShader(GLenum type, const char* source) {
    int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
        return -1;
    }
    return shader;
}

int createShaderProgram() {
    int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("Shader program linking error: %s\n", infoLog);
        return -1;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void setupBuffers(unsigned int* VAO, unsigned int* VBO, unsigned int* EBO) {
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  // Bottom-left back
        0.5f, -0.5f, -0.5f,   // Bottom-right back
        0.5f,  0.5f, -0.5f,   // Top-right back
        -0.5f,  0.5f, -0.5f,  // Top-left back
        -0.5f, -0.5f,  0.5f,  // Bottom-left front
        0.5f, -0.5f,  0.5f,   // Bottom-right front
        0.5f,  0.5f,  0.5f,   // Top-right front
        -0.5f,  0.5f,  0.5f   // Top-left front
    };

    unsigned int indices[] = {
        // Back face
        0, 1, 2,
        2, 3, 0,
        // Front face
        4, 5, 6,
        6, 7, 4,
        // Left face
        0, 3, 7,
        7, 4, 0,
        // Right face
        1, 5, 6,
        6, 2, 1,
        // Bottom face
        0, 1, 5,
        5, 4, 0,
        // Top face
        3, 2, 6,
        6, 7, 3
    };

    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
    glGenBuffers(1, EBO);

    glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void updateMatrices(int shaderProgram) {
    mat4 model, view, projection;
    glm_mat4_identity(model);
    glm_mat4_identity(view);
    glm_mat4_identity(projection);

    glm_translate(model, (vec3){0.0f, 0.0f, 0.0f});
    glm_lookat(cameraPos, (vec3){cameraPos[0] + cameraFront[0], cameraPos[1] + cameraFront[1], cameraPos[2] + cameraFront[2]}, cameraUp, view);
    glm_perspective(glm_rad(45.0f), 800.0f / 600.0f, 0.1f, 100.0f, projection);

    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat*)model);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat*)view);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (const GLfloat*)projection);
}

int main(void) {
    if (!glfwInit()) {
        printf("GLFW initialization error\n");
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGLTest", NULL, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() != GLEW_OK) {
        printf("GLEW initialization error\n");
        return -1;
    }

    int shaderProgram = createShaderProgram();
    if (shaderProgram == -1) return -1;

    unsigned int VAO, VBO, EBO;
    setupBuffers(&VAO, &VBO, &EBO);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.0f, 0.74f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        updateMatrices(shaderProgram);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
