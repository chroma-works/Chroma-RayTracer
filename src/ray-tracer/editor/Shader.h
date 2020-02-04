#pragma once

#include <ray-tracer/editor/Logger.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>

namespace CH_Editor
{
	class Shader
	{
	public:
		static inline Shader* ReadAndBuildShaderFromFile
		(const std::string& vertex_path, const std::string& fragment_path)
		{
			return new Shader(vertex_path, fragment_path, READ_FILE_PATH);
		}

		static inline Shader* ReadAndBuildShaderFromSource
		(const std::string& vertex_src, const std::string& fragment_src)
		{
			return new Shader(vertex_src, fragment_src);
		}
		~Shader();

		void Bind() const;
		void Unbind() const;

		//Apply RULE OF THREE
		//Delete the copy constructor/assignment.
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		Shader(Shader&& other)
		{
			m_renderer_id = other.m_renderer_id;
			other.m_renderer_id = 0;
		}

		Shader& operator=(Shader&& other)
		{
			if (this != &other)
			{
				Unbind();
				std::swap(m_renderer_id, other.m_renderer_id);
			}
		}

	private:
		static const bool READ_FILE_PATH = true;
		Shader(const std::string& vertex_shader, const std::string& fragment_shader, bool read_from_file = false);
		unsigned int CompileShader(unsigned int type, const std::string& source);

		uint32_t m_renderer_id;
	};
}