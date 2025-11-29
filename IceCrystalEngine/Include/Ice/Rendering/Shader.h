#pragma once // this prevents multiple definitions of the class
#ifndef SHADER_H // prevents the same header file from being included more than once

#define SHADER_H // definition

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {

	void InitializeShader();
	
public:
	
	std::string VertexShaderPath = "{ASSET_DIR}Shaders/default.vert"; // i dont really need these, but they might come in handy eventually
	std::string FragmentShaderPath = "{ASSET_DIR}Shaders/default.frag";
	std::string GeometryShaderPath = "";

	std::int32_t Handle;

	Shader();
	Shader(std::string shaderName); // Loads {shaderName}.vert/.frag/.geom
	Shader(std::string vertexShaderPath, std::string fragmentShaderPath);
	Shader(std::string vertexShaderPath, std::string fragmentShaderPath, std::string geometryShaderPath);

    ~Shader();

	void Use();


    // utility uniform functions
    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec2(const std::string& name, const glm::vec2& value);
    void setVec2(const std::string& name, float x, float y);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec3(const std::string& name, float x, float y, float z);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setVec4(const std::string& name, float x, float y, float z, float w);
    void setMat2(const std::string& name, const glm::mat2& mat);
    void setMat3(const std::string& name, const glm::mat3& mat);
    void setMat4(const std::string& name, const glm::mat4& mat);

	
};

#endif // ends the guard