#pragma once
#include <ray-tracer/main/Ray.h>

namespace CHR
{
	class AccelerationStructure
	{
	public:
		virtual bool Intersect(const Ray& ray, IntersectionData* intersection_data) const = 0;
		virtual bool IntersectP(const Ray& ray) const = 0;

		// Total number of bytes required for storing
		// this acceleration structure in memory.
		virtual int GetSizeBytes() = 0;
	};
}