#include "Shader.h"
#include <thirdparty\glad\include\glad\glad.h>


#include <fstream>
#include <thirdparty/glm/glm/glm.hpp>

namespace Chroma
{
	const std::string Shader::MODEL_SH = "u_Model";
	const std::string Shader::VIEW_SH = "u_View";
	const std::string Shader::PROJ_SH = "u_Proj";
	const std::string Shader::NORM_MAT_SH = "u_NormalMat";
	const std::string Shader::CAM_POS_SH = "u_CameraPos";
	const std::string Shader::MATERIAL_SH = "u_Material";

	Shader::Shader(const std::string& vertex_shader_data, const std::string& fragment_shader_data, bool read_from_file)
	{
		std::string vertex_shader_src;
		std::string fragment_shader_src;
		if (read_from_file)
		{
			std::ifstream vertex_shader_stream(vertex_shader_data, std::ios::in);
			if (vertex_shader_stream.is_open())
			{
				std::stringstream sstr;
				sstr << vertex_shader_stream.rdbuf();
				vertex_shader_src = sstr.str();
				vertex_shader_stream.close();
			}
			else
			{
				CH_ASSERT(false, "Cannot to open vertex shader")
			}

			std::ifstream fragment_shader_stream(fragment_shader_data, std::ios::in);
			if (fragment_shader_stream.is_open())
			{
				std::stringstream sstr;
				sstr << fragment_shader_stream.rdbuf();
				fragment_shader_src = sstr.str();
				fragment_shader_stream.close();
			}
			else
			{
				CH_ASSERT(false, "Cannot to open fragment shader")
			}
		}
		else
		{
			vertex_shader_src = vertex_shader_data;
			fragment_shader_src = fragment_shader_data;
		}
		m_renderer_id = glCreateProgram();
		unsigned int vertex_shader = CompileShader(GL_VERTEX_SHADER, vertex_shader_src);
		unsigned int fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fragment_shader_src);

		glAttachShader(m_renderer_id, vertex_shader);
		glAttachShader(m_renderer_id, fragment_shader);
		glLinkProgram(m_renderer_id);
		glValidateProgram(m_renderer_id);


		glDetachShader(m_renderer_id, vertex_shader);
		glDetachShader(m_renderer_id, fragment_shader);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		CH_INFO("Shaders compiled and linked successfully");
	}

	Shader::~Shader()
	{
		glDeleteProgram(m_renderer_id);
	}

	void Shader::Bind() const
	{
		glUseProgram(m_renderer_id);
	}

	void Shader::Unbind() const
	{
		glUseProgram(0);
	}

	void Shader::CreateUniform(std::string name, ShaderDataType type, void* data)
	{
		Uniform uniform(name, type);
		uniform.data = data;
		AddUniform(uniform);
	}

	void Shader::CreateUniform(Chroma::Material* mat)
	{
		Uniform ambient(mat->shader_var_name + ".ambient", ShaderDataType::Float3);
		ambient.data = (void*)&mat->m_ambient;
		Uniform diffuse(mat->shader_var_name + ".diffuse", ShaderDataType::Float3);
		diffuse.data = (void*)&mat->m_diffuse;
		Uniform specular(mat->shader_var_name + ".specular", ShaderDataType::Float3);
		specular.data = (void*)&mat->m_specular;
		Uniform shininess(mat->shader_var_name + ".shininess", ShaderDataType::Float);
		shininess.data = (void*)&mat->m_shininess;

		AddUniform(ambient);
		AddUniform(diffuse);
		AddUniform(specular);
		AddUniform(shininess);
	}

	void Shader::AddLight(std::shared_ptr<DirectionalLight> lig)
	{
		if (num_dir_lights < MAX_NUM_LIGHTS)
		{
			Uniform direction(lig->m_shader_var_name + "[" + std::to_string(num_dir_lights) + "].direction", ShaderDataType::Float3);
			direction.data = (void*)&lig->direction;
			AddUniform(direction);

			Uniform ambient(lig->m_shader_var_name + "[" + std::to_string(num_dir_lights) + "].ambient", ShaderDataType::Float3);
			ambient.data = (void*)&lig->m_ambient;
			AddUniform(ambient);

			Uniform diffuse(lig->m_shader_var_name + "[" + std::to_string(num_dir_lights) + "].diffuse", ShaderDataType::Float3);
			diffuse.data = (void*)&lig->m_diffuse;
			AddUniform(diffuse);

			Uniform specular(lig->m_shader_var_name + "[" + std::to_string(num_dir_lights) + "].specular", ShaderDataType::Float3);
			specular.data = (void*)&lig->m_specular;
			AddUniform(specular);
			//m_dir_lights.push_back(lig);
			num_dir_lights++;
			//update # of lights
			Bind();
			int location = glGetUniformLocation(m_renderer_id, "u_NumDirLights");//TODO: make the name not hardcoded
			glUniform1i(location, num_dir_lights);
			Unbind();
		}
		else
			CH_WARN("Adding ligth failed:\n\t Maximum number of DirectionalLights(" + std::to_string(MAX_NUM_LIGHTS) + ") has been reached");
	}

	void Shader::AddLight(std::shared_ptr<PointLight> lig)
	{
		if (num_point_lights < MAX_NUM_LIGHTS)
		{
			Uniform position(lig->m_shader_var_name + "[" + std::to_string(num_point_lights) + "].position", ShaderDataType::Float3);
			position.data = (void*)&lig->position;
			AddUniform(position);

			Uniform constant(lig->m_shader_var_name + "[" + std::to_string(num_point_lights) + "].constant", ShaderDataType::Float);
			constant.data = (void*)&lig->constant; //i hate you so fucking much
			AddUniform(constant);

			Uniform linear(lig->m_shader_var_name + "[" + std::to_string(num_point_lights) + "].linear", ShaderDataType::Float);
			linear.data = (void*)&lig->linear;
			AddUniform(linear);

			Uniform quadratic(lig->m_shader_var_name + "[" + std::to_string(num_point_lights) + "].quadratic", ShaderDataType::Float);
			quadratic.data = (void*)&lig->quadratic;
			AddUniform(quadratic);

			Uniform ambient(lig->m_shader_var_name + "[" + std::to_string(num_point_lights) + "].ambient", ShaderDataType::Float3);
			ambient.data = (void*)&lig->m_ambient;
			AddUniform(ambient);

			Uniform diffuse(lig->m_shader_var_name + "[" + std::to_string(num_point_lights) + "].diffuse", ShaderDataType::Float3);
			diffuse.data = (void*)&lig->m_diffuse;
			AddUniform(diffuse);

			Uniform specular(lig->m_shader_var_name + "[" + std::to_string(num_point_lights) + "].specular", ShaderDataType::Float3);
			specular.data = (void*)&lig->m_specular;
			AddUniform(specular);
			//m_point_lights.push_back(lig);
			num_point_lights++;
			//update # of lights
			Bind();
			int location = glGetUniformLocation(m_renderer_id, "u_NumPointLights");//TODO: make the name not hardcoded
			glUniform1i(location, num_point_lights);
			Unbind();
		}
		else
			CH_WARN("Adding ligth failed:\n\t Maximum number of PointLights(" + std::to_string(MAX_NUM_LIGHTS) + ") has been reached");
	}

	void Shader::AddLight(std::shared_ptr<SpotLight> lig)
	{
		if (num_spot_lights < MAX_NUM_LIGHTS)
		{
			Uniform position(lig->m_shader_var_name + "[" + std::to_string(num_spot_lights) + "].position", ShaderDataType::Float3);
			position.data = (void*)&lig->position;
			AddUniform(position);

			Uniform direction(lig->m_shader_var_name + "[" + std::to_string(num_spot_lights) + "].direction", ShaderDataType::Float3);
			direction.data = (void*)&lig->direction;
			AddUniform(direction);

			Uniform cutOff(lig->m_shader_var_name + "[" + std::to_string(num_spot_lights) + "].cutOff", ShaderDataType::Float);
			cutOff.data = (void*)&lig->fall_off;
			AddUniform(cutOff);

			Uniform outerCutOff(lig->m_shader_var_name + "[" + std::to_string(num_spot_lights) + "].outerCutOff", ShaderDataType::Float);
			outerCutOff.data = (void*)&lig->cut_off;
			AddUniform(outerCutOff);

			Uniform constant(lig->m_shader_var_name + "[" + std::to_string(num_spot_lights) + "].constant", ShaderDataType::Float);
			constant.data = (void*)&lig->constant;
			AddUniform(constant);

			Uniform linear(lig->m_shader_var_name + "[" + std::to_string(num_spot_lights) + "].linear", ShaderDataType::Float);
			linear.data = (void*)&lig->linear;
			AddUniform(linear);

			Uniform quadratic(lig->m_shader_var_name + "[" + std::to_string(num_spot_lights) + "].quadratic", ShaderDataType::Float);
			quadratic.data = (void*)&lig->quadratic;
			AddUniform(quadratic);

			Uniform ambient(lig->m_shader_var_name + "[" + std::to_string(num_spot_lights) + "].ambient", ShaderDataType::Float3);
			ambient.data = (void*)&lig->m_ambient;
			AddUniform(ambient);

			Uniform diffuse(lig->m_shader_var_name + "[" + std::to_string(num_spot_lights) + "].diffuse", ShaderDataType::Float3);
			diffuse.data = (void*)&lig->m_diffuse;
			AddUniform(diffuse);

			Uniform specular(lig->m_shader_var_name + "[" + std::to_string(num_spot_lights) + "].specular", ShaderDataType::Float3);
			specular.data = (void*)&lig->m_specular;
			AddUniform(specular);
			//m_spot_lights.push_back(lig);
			num_spot_lights++;
			//update # of lights
			Bind();
			int location = glGetUniformLocation(m_renderer_id, "u_NumSpotLights");//TODO: make the name not hardcoded
			glUniform1i(location, num_spot_lights);
			Unbind();
		}
		else
			CH_WARN("Adding ligth failed:\n\t Maximum number of Spotligths(" + std::to_string(MAX_NUM_LIGHTS) + ") has been reached");
	}

	void Shader::UpdateUniforms()
	{
		Bind();
		for (int i = 0; i < m_uniforms.size(); i++)
		{
			Uniform uniform = m_uniforms[i];
			switch (uniform.data_type)
			{
			case ShaderDataType::Bool:
				glUniform1i(uniform.shader_location, **(bool**) & (uniform).data);
				break;
			case ShaderDataType::Float4:
				glUniform4f(uniform.shader_location, (*((float**)&uniform.data))[0],
					(*((float**)&uniform.data))[1], (*((float**)&uniform.data))[2],
					(*((float**)&uniform.data))[3]);
				break;
			case ShaderDataType::Float3:
				glUniform3f(uniform.shader_location, (*((float**)&uniform.data))[0],
					(*((float**)&uniform.data))[1], (*((float**)&uniform.data))[2]);
				break;
			case ShaderDataType::Float2:
				glUniform2f(uniform.shader_location, (*((float**)&uniform.data))[0],
					(*((float**)&uniform.data))[1]);
				break;
			case ShaderDataType::Float:
				glUniform1f(uniform.shader_location, **(float**) & (uniform).data);
				break;
			case ShaderDataType::Int4:
				glUniform4f(uniform.shader_location, (*((int**)&uniform.data))[0],
					(*((int**)&uniform.data))[1], (*((int**)&uniform.data))[2],
					(*((int**)&uniform.data))[3]);
				break;
			case ShaderDataType::Int3:
				glUniform3f(uniform.shader_location, (*((int**)&uniform.data))[0],
					(*((int**)&uniform.data))[1], (*((int**)&uniform.data))[2]);
				break;
			case ShaderDataType::Int2:
				glUniform2f(uniform.shader_location, (*((int**)&uniform.data))[0],
					(*((int**)&uniform.data))[1]);
				break;
			case ShaderDataType::Int:
				glUniform1f(uniform.shader_location, **(int**) & (uniform).data);
				break;
			case ShaderDataType::Mat3:
				glUniformMatrix3fv(uniform.shader_location, 1, GL_FALSE, (((float**)&uniform.data))[0]);
				break;
			case ShaderDataType::Mat4:
				glUniformMatrix4fv(uniform.shader_location, 1, GL_FALSE, (((float**)&uniform.data))[0]);
				break;
			default:
				CH_ERROR("Unknown type!");
			}
		}
		//Unbind();
	}

	unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
	{
		unsigned int id = glCreateShader(type);
		const char* src = source.c_str();
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		int result;
		glGetShaderiv(id, GL_COMPILE_STATUS, &result);
		if (result == GL_FALSE)
		{
			std::stringstream ss;
			int length;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
			char* message = (char*)alloca(length * sizeof(char));
			glGetShaderInfoLog(id, length, &length, message);

			ss << "Cannot compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader:\n" << message;
			glDeleteShader(id);

			CH_ASSERT(false, ss.str());
			return -1;
		}

		return id;
	}

	void Shader::AddUniform(Uniform uniform)
	{
		for (int i = 0; i < m_uniforms.size(); i++)
		{
			if (uniform.shader_var_name.compare(m_uniforms[i].shader_var_name) == 0)
			{
				CH_WARN("Uniform " + uniform.shader_var_name + " is already added");
				return;
			}
		}
		int location = glGetUniformLocation(m_renderer_id, uniform.shader_var_name.c_str());
		if (location == -1)
			CH_WARN("Uniform " + uniform.shader_var_name + " does not exist!");
		else
		{
			uniform.shader_location = location;
			m_uniforms.push_back(uniform);
		}
	}

}