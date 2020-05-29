#pragma once

#include <string>
#include "thirdparty/glm/glm/glm.hpp"

namespace CHR
{
    class Texture
    {
    public:
        enum TextureWrap{ EDGE_CLAMP = 0x812F, MIRRORED_REPEAT = 0x8370, REPEAT = 0x2901};

        Texture();
        Texture(const std::string& path, TextureWrap wrap = EDGE_CLAMP);
        Texture(const Texture& other);
        Texture &operator=(Texture rhs);
        ~Texture();

        void Bind(unsigned int slot = 0) const;
        void Unbind() const;

        inline int GetWidth() const { return m_width; }
		inline int GetHeigth() const { return m_height; }

		inline std::string GetFilePath() const { return m_filepath; }


		glm::vec4 SampleAt(glm::ivec2 p);

    private:
        unsigned int m_renderer_id;
        std::string m_filepath;
        unsigned char* m_localbuffer;
        int m_width, m_height, m_BPP;
        TextureWrap m_t_wrap;

		float* m_hdr_localbuffer;
		bool m_hdr = false;
    };
}