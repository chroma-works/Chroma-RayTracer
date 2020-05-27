#include "Image.h"

#include <algorithm>

#include <ray-tracer/editor/Logger.h>
#include <thirdparty/glm/glm/glm.hpp>
#include <thirdparty/glm/glm/common.hpp>
#include <thirdparty/glm/glm/gtx/color_space.hpp>
#include <thirdparty/glm/glm/gtx/component_wise.hpp>
#include <thirdparty/glm/glm/exponential.hpp>

//#define TINYEXR_IMPLEMENTATION
#include "thirdparty/tinyexr/tinyexr.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <thirdparty/stb_image/stb_image_write.h>

namespace Chroma
{
	float luminosity(glm::vec3 color)
	{
		return color.r + color.g * 4.590f + color.b * 0.060f;
	}

	bool SaveEXR(const float* rgb, int width, int height, const char* outfilename) {

		EXRHeader header;
		InitEXRHeader(&header);

		EXRImage image;
		InitEXRImage(&image);

		image.num_channels = 3;

		std::vector<float> images[3];
		images[0].resize(width * height);
		images[1].resize(width * height);
		images[2].resize(width * height);

		// Split RGBRGBRGB... into R, G and B layer
		for (int i = 0; i < width * height; i++) {
			images[0][i] = rgb[3 * i + 0];
			images[1][i] = rgb[3 * i + 1];
			images[2][i] = rgb[3 * i + 2];
		}

		float* image_ptr[3];
		image_ptr[0] = &(images[2].at(0)); // B
		image_ptr[1] = &(images[1].at(0)); // G
		image_ptr[2] = &(images[0].at(0)); // R

		image.images = (unsigned char**)image_ptr;
		image.width = width;
		image.height = height;

		header.num_channels = 3;
		header.channels = (EXRChannelInfo*)malloc(sizeof(EXRChannelInfo) * header.num_channels);
		// Must be (A)BGR order, since most of EXR viewers expect this channel order.
		strncpy(header.channels[0].name, "B", 255); header.channels[0].name[strlen("B")] = '\0';
		strncpy(header.channels[1].name, "G", 255); header.channels[1].name[strlen("G")] = '\0';
		strncpy(header.channels[2].name, "R", 255); header.channels[2].name[strlen("R")] = '\0';

		header.pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
		header.requested_pixel_types = (int*)malloc(sizeof(int) * header.num_channels);
		for (int i = 0; i < header.num_channels; i++) {
			header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
			header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
		}

		const char* err = NULL; // or nullptr in C++11 or later.
		int ret = SaveEXRImageToFile(&image, &header, outfilename, &err);
		if (ret != TINYEXR_SUCCESS) {
			//fprintf(stderr, "Save EXR err: %s\n", err);
			FreeEXRErrorMessage(err); // free's buffer for an error message
			return ret;
		}

		free(header.channels);
		free(header.pixel_types);
		free(header.requested_pixel_types);

	}

	Image::Image(int width, int height, bool is_hdr)
		: m_width(width), m_height(height), m_hdr(is_hdr)
	{
		m_ldr_pixels = new glm::u8vec3[m_width * m_height];
		if(m_hdr)
			m_hdr_pixels = new glm::vec3[m_width * m_height];
	}
	Image::~Image()
	{
		delete[] m_ldr_pixels;
		if (m_hdr)
			delete[] m_hdr_pixels;
	}
	/*void Image::SetHDR(bool is_hdr)
	{
		m_hdr = is_hdr;
		if (m_hdr)
		{
			if(!m_hdr_pixels)
				m_hdr_pixels = new glm::vec3[m_width * m_height];
		}
		else
		{
			if (m_hdr_pixels)
				delete[] m_hdr_pixels;
		}
	}*/
	glm::u8vec3* Image::GetPixels() const
	{
		return m_ldr_pixels;
	}
	void Image::ToneMap(float key_v, float burn_per, float satur, float gamma)
	{
		float tmp = 0.0f;

		std::vector<float> luminances(m_width * m_height);
		//Calculate L_w_hat
		for (int i = 0; i < m_width * m_height; i++)
		{
			luminances[i] = glm::luminosity(m_hdr_pixels[i]);
			tmp += std::logf( 0.00001f + luminances[i]);
		}
		float l_w_hat = expf(tmp / ((float)m_width * m_height));

		//sort luminaces to find L_white
		std::sort(luminances.begin(), luminances.end());
		float l_white = luminances[(m_width * m_height) * (1.0f - burn_per / 100.0f)];

		for (int i = 0; i < m_width * m_height; i++)
		{
			float l_scaled = key_v / l_w_hat * (glm::luminosity(m_hdr_pixels[i]));

			float l_out = ( l_scaled * (1 + l_scaled / (l_white * l_white)) );

			glm::vec3 color = glm::clamp(l_out * glm::pow(m_hdr_pixels[i] / glm::luminosity(m_hdr_pixels[i]), glm::vec3(1,1,1) * satur)
				,0.0f,1.0f);

			//gamma correction
			m_ldr_pixels[i] = 255.0f * glm::pow(color, glm::vec3(1,1,1) / gamma);
			//m_ldr_pixels[i] = m_hdr_pixels[i];
		}
		
	}
	void Image::SetPixel(int x, int y, const glm::vec3& pixel)
	{
		if (m_hdr)
			m_hdr_pixels[y * m_width + x] = pixel;
		else
		{
			m_ldr_pixels[y * m_width + x] = glm::clamp(pixel, 0.0f, 255.0f);
		}
	}
	void Image::SaveToDisk(const char* file_name) const
	{
		std::string s = file_name;
		size_t index = s.find_last_of(".");
		std::string token = s.substr(0,index);
		std::string tmp = token;

		stbi_write_png(tmp.append(".png").c_str(), m_width, m_height, 3, m_ldr_pixels, 3 * m_width);
		CH_TRACE("Image Saved to" + tmp);
		if (m_hdr)
		{
			tmp = token;
			SaveEXR(&m_hdr_pixels[0].x, m_width, m_height, tmp.append(".exr").c_str());
			CH_TRACE("Image Saved to" + tmp);
		}
	}
}