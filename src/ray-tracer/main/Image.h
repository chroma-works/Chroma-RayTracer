#pragma once
#include <thirdparty/glm/glm/glm.hpp>

namespace CHR
{
	class Image
	{
	public:
		Image(int width, int height, bool is_hdr = false);
		~Image();


		glm::u8vec3* GetPixels() const;

		//void SetHDR(bool is_hdr);
		inline bool IsHDR() { return m_hdr; }

		void ToneMap(float key_v, float burn, float satur, float gamma);
		inline void Clamp(float l, float h)
		{
			for (int i = 0; i < m_width * m_height; i++)
			{
				m_ldr_pixels[i] = glm::clamp(m_hdr_pixels[i], l, h);
			}
		}
		void SetPixel(int x, int y, const glm::vec3& pixel);
		void SaveToDisk(const char* file_name) const;



	private:
		const int m_width;
		const int m_height;
		bool m_hdr = false;
		glm::u8vec3* m_ldr_pixels = nullptr;
		glm::vec3* m_hdr_pixels = nullptr;

	};

}