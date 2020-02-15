#pragma once
#include <thirdparty/glm/glm/glm.hpp>

namespace Chroma
{
	class Image
	{
	public:
		Image(int width, int height);
		~Image();

		glm::u8vec3* GetPixels() const;

		void SetPixel(int x, int y, const glm::u8vec3& pixel);
		void SaveToDisk(const char* file_name) const;



	private:
		const int m_width;
		const int m_height;
		glm::u8vec3* m_pixels = nullptr;
	};

}