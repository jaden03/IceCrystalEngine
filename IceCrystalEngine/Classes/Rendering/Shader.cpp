#include <Ice/Rendering/Shader.h>

#include <Ice/Utils/FileUtil.h>
#include <iostream>
#include <fstream>

#include "Ice/Core/LightingManager.h"

std::int32_t Handle;

Shader::Shader() : Handle(0) // This will just use default.vert and default.frag
{
    InitializeShader();
}
Shader::Shader(std::string shaderName)
{
    if (!FileUtil::FileExists(shaderName + ".vert"))
    {
        std::cerr << "Failed to load vertex shader: " << shaderName << "\n";
        abort();
    }
    VertexShaderPath = shaderName + ".vert";

    if (!FileUtil::FileExists(shaderName + ".frag"))
    {
        std::cerr << "Failed to load fragment shader: " << shaderName << "\n";
        abort();
    }
    FragmentShaderPath = shaderName + ".frag";

    if (FileUtil::FileExists(shaderName + ".geom"))
        GeometryShaderPath = shaderName + ".geom";

    InitializeShader();
}
Shader::Shader(std::string vertexShaderPath, std::string fragmentShaderPath)
{
	VertexShaderPath = vertexShaderPath;
	FragmentShaderPath = fragmentShaderPath;

    InitializeShader();
}
Shader::Shader(std::string vertexShaderPath, std::string fragmentShaderPath, std::string geometryShaderPath)
{
    VertexShaderPath = vertexShaderPath;
    FragmentShaderPath = fragmentShaderPath;
    GeometryShaderPath = geometryShaderPath;

    InitializeShader();
}


void Shader::Use()
{
	glUseProgram(Handle);
}

Shader::~Shader()
{
    if (Handle != 0)
    {
        glDeleteProgram(Handle);
        Handle = 0;
    }
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


    // Read the geometry shader file
    unsigned int geometryShader = 0;
    if (!GeometryShaderPath.empty())
    {
        std::string geometryShaderFileContents = FileUtil::ReadFile(GeometryShaderPath);
        const char* geometryShaderSource = geometryShaderFileContents.c_str();

        // geometry shader
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &geometryShaderSource, NULL);
        glCompileShader(geometryShader);
        
        glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    // link shaders
    Handle = glCreateProgram();
    glAttachShader(Handle, vertexShader);
    glAttachShader(Handle, fragmentShader);
    if (geometryShader != 0)
    {
        glAttachShader(Handle, geometryShader);
    }
    glLinkProgram(Handle);

    glGetProgramiv(Handle, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(Handle, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << " " << VertexShaderPath << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(geometryShader);

    // Set default values
    Use();
    setInt("directionalShadowMap", LightingManager::directionalShadowMapUnit);
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