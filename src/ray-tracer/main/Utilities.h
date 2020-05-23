#pragma once
#include <random>
#include <limits>

#include <thirdparty\glm\glm\glm.hpp>
#include <thirdparty\glm\glm\gtx\norm.hpp>
#include <thirdparty\glm\glm\gtx\component_wise.hpp>


namespace Utils
{
	inline glm::vec2 SampleUnitSquare()
	{
		std::random_device rd;  //Will be used to obtain a seed for the random number engine
		std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
		std::uniform_real_distribution<> dis(0.0, 1.0);
		return { dis(gen), dis(gen) };
	}
	inline glm::vec2 SampleUnitDisk()
	{
		std::random_device rd;  //Will be used to obtain a seed for the random number engine
		std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
		std::uniform_real_distribution<double> dis(0.0, 1.0f);
		const double pi2 = 2.0f * glm::pi<double>();
		double theta = pi2 * dis(gen);
		double r = dis(gen);

		return { r * cos(theta), r * sin(theta) };
	}

	inline float RandFloat(float l = 0.0f, float u = 1.0f)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(l, u);

		return dis(gen);
	}

	inline glm::vec3 CalculateNonColinearTo(glm::vec3 r)
	{
		glm::vec3 r_abs = glm::abs(r);
		//Find smallest r component
		int ind = r_abs.x > r_abs.y ? (r_abs.y > r_abs.z ? 2 : 1) : (r_abs.x > r_abs.z ? 2 : 0);
		glm::vec3 r_prime = r;
		r_prime[ind] = 1.0f;
		return r_prime;
	}
}
