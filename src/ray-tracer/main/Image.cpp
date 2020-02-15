#include "Image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <thirdparty/stb_image/stb_image_write.h>

namespace Chroma
{
	Image::Image(int width, int height)
		: m_width(width), m_height(height)
	{
		m_pixels = new glm::u8vec3[m_width * m_height];
	}
	Image::~Image()
	{
		delete[] m_pixels;
	}
	glm::u8vec3* Image::GetPixels() const
	{
		return m_pixels;
	}
	void Image::SetPixel(int x, int y, const glm::u8vec3& pixel)
	{
		m_pixels[y * m_width + x] = pixel;
	}
	void Image::SaveToDisk(const char* file_name) const
	{
		stbi_write_png(file_name, m_width, m_height, 3, m_pixels, 3 * m_width);
	}
}