#include "Texture.h"
#include <thirdparty/glad/include/glad/glad.h>

#include <thirdparty/tinyexr/tinyexr.h>

#define STB_IMAGE_IMPLEMENTATION
#include <thirdparty/stb_image/stb_image.h>
#include <ray-tracer\editor\Logger.h>

namespace Chroma
{
    Texture::Texture()
    {}

    Texture::Texture(const std::string & path, TextureWrap wrap)
        : m_renderer_id(0), m_filepath(path), m_localbuffer(nullptr), m_width(0), m_height(0), m_BPP(0), m_t_wrap(wrap)
    {
		size_t index = path.find_last_of(".");
		std::string token = path.substr(index, index+3);
		m_hdr = token.compare(".exr")==0;
		if (!m_hdr)
		{
			stbi_set_flip_vertically_on_load(false);
			m_localbuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_BPP, 4);

			glGenTextures(1, &m_renderer_id);
			glBindTexture(GL_TEXTURE_2D, m_renderer_id);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_localbuffer);
			glBindTexture(GL_TEXTURE_2D, 0);

			/*if (m_localbuffer)
				stbi_image_free(m_localbuffer);*/
		}
		else
		{
			const char* err = NULL; // or nullptr in C++11

			int ret = LoadEXR(&m_hdr_localbuffer, &m_width, &m_height, m_filepath.c_str(), &err);

			if (ret != TINYEXR_SUCCESS) {
				if (err) {
					CH_WARN(err);
					FreeEXRErrorMessage(err); // release memory of error message.
				}
			}
		}
    }

    Texture& Texture::operator=(Texture rhs)
    {
        if (this != &rhs)
        {
            m_BPP = rhs.m_BPP;
            m_filepath = rhs.m_filepath;
            m_height = rhs.m_height;
            //m_localbuffer = nullptr;
            m_renderer_id = rhs.m_renderer_id;
            //m_uniform_name = rhs.m_uniform_name;
            m_width = rhs.m_width;
            m_t_wrap = rhs.m_t_wrap;

			m_hdr_localbuffer = rhs.m_hdr_localbuffer;
			m_hdr = rhs.m_hdr;

			if (!m_hdr)
			{

				stbi_set_flip_vertically_on_load(false);
				m_localbuffer = stbi_load(m_filepath.c_str(), &m_width, &m_height, &m_BPP, 4);

				glGenTextures(1, &m_renderer_id);
				glBindTexture(GL_TEXTURE_2D, m_renderer_id);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, rhs.m_t_wrap);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, rhs.m_t_wrap);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_localbuffer);
				glBindTexture(GL_TEXTURE_2D, 0);

				/*if (m_localbuffer)
					stbi_image_free(m_localbuffer);*/
			}

        }
        return *this;
    }

    Texture::Texture(const Texture & other)
        :m_renderer_id(other.m_renderer_id),
        m_filepath(other.m_filepath),
        //m_localbuffer(other.m_localbuffer),
        m_width(other.m_width),
        m_height(other.m_height),
        m_BPP(other.m_BPP),
        m_t_wrap(other.m_t_wrap)/*,
        m_uniform_name(other.m_uniform_name)*/
    {
		if (!m_hdr)
		{
			if (!m_hdr)
			{
				stbi_set_flip_vertically_on_load(false);
				m_localbuffer = stbi_load(m_filepath.c_str(), &m_width, &m_height, &m_BPP, 4);

				glGenTextures(1, &m_renderer_id);
				glBindTexture(GL_TEXTURE_2D, m_renderer_id);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, other.m_t_wrap);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, other.m_t_wrap);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_localbuffer);
				glBindTexture(GL_TEXTURE_2D, 0);
				/*if (m_localbuffer)
					stbi_image_free(m_localbuffer);*/
			}

		}
    }

    Texture::~Texture()
    {
		if(!m_hdr)
			glDeleteTextures(1, &m_renderer_id);
    }

    void Texture::Bind(unsigned int slot) const
    {
		if (!m_hdr)
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, m_renderer_id);
		}
    }

    void Texture::Unbind() const
    {
		if (!m_hdr)
			glBindTexture(GL_TEXTURE_2D, 0);
    }

	glm::vec4 Texture::SampleAt(const glm::ivec2 p_prime)
	{
		
		glm::ivec2 p = glm::clamp(p_prime, { 0,0 }, { m_width - 1, m_height - 1 });
		if (!m_hdr)
		{
			const stbi_uc* pixelOffset = m_localbuffer + (p.x + m_width * p.y) * 4;
			unsigned int r = pixelOffset[0];
			unsigned int g = pixelOffset[1];
			unsigned int b = pixelOffset[2];
			unsigned int a = m_BPP >= 4 ? pixelOffset[3] : 0xff;

			return glm::vec4(r, g, b, a);
		}
		else
		{
			const float* pixelOffset = m_hdr_localbuffer + (p.x + m_width * p.y) * 4;
			float r = pixelOffset[0];
			float g = pixelOffset[1];
			float b = pixelOffset[2];
			float a = pixelOffset[3];

			return glm::vec4(r, g, b, a);
		}
	}
}