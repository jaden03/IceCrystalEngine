#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <Ice/Rendering/Shader.h>
#include <Ice/Utils/FileUtil.h>

std::int32_t Handle;

Shader::Shader() // default constructer
{
    InitializeShader();
}

Shader::Shader(std::string vertexShaderPath, std::string fragmentShaderPath)
{
	VertexShaderPath = vertexShaderPath;
	FragmentShaderPath = fragmentShaderPath;

    InitializeShader();
}

void Shader::Use()
{
	glUseProgram(Handle);
}

Shader::~Shader()
{
	glDeleteProgram(Handle);
}

void Shader::InitializeShader()
{
    // Read the vertex shader file
    std::string vertexShaderFileContents = FileUtil::ReadFile(VertexShaderPath);
    const char* vertexShaderSource = vertexShaderFileContents.c_str();

    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


	// Read the fragment shader file
    std::string fragmentShaderFileContents = FileUtil::ReadFile(FragmentShaderPath);
    const char* fragmentShaderSource = fragmentShaderFileContents.c_str();

    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // link shaders
    Handle = glCreateProgram();
    glAttachShader(Handle, vertexShader);
    glAttachShader(Handle, fragmentShader);
    glLinkProgram(Handle);

    glGetProgramiv(Handle, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(Handle, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}


// utility uniform functions
void Shader::setBool(const std::string& name, bool value)
{
    glUniform1i(glGetUniformLocation(Handle, name.c_str()), (int)value);
}
void Shader::setInt(const std::string& name, int value)
{
    glUniform1i(glGetUniformLocation(Handle, name.c_str()), value);
}
void Shader::setFloat(const std::string& name, float value)
{
    glUniform1f(glGetUniformLocation(Handle, name.c_str()), value);
}
void Shader::setVec2(const std::string& name, const glm::vec2& value)
{
    glUniform2fv(glGetUniformLocation(Handle, name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string& name, float x, float y)
{
    glUniform2f(glGetUniformLocation(Handle, name.c_str()), x, y);
}
void Shader::setVec3(const std::string& name, const glm::vec3& value)
{
    glUniform3fv(glGetUniformLocation(Handle, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, float x, float y, float z)
{
    glUniform3f(glGetUniformLocation(Handle, name.c_str()), x, y, z);
}
void Shader::setVec4(const std::string& name, const glm::vec4& value)
{
    glUniform4fv(glGetUniformLocation(Handle, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, float x, float y, float z, float w)
{
    glUniform4f(glGetUniformLocation(Handle, name.c_str()), x, y, z, w);
}
void Shader::setMat2(const std::string& name, const glm::mat2& mat)
{
    glUniformMatrix2fv(glGetUniformLocation(Handle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat3(const std::string& name, const glm::mat3& mat)
{
    glUniformMatrix3fv(glGetUniformLocation(Handle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat4(const std::string& name, const glm::mat4& mat)
{
    glUniformMatrix4fv(glGetUniformLocation(Handle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}