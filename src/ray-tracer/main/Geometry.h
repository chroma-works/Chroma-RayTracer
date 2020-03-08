#pragma once

#include <algorithm>
#include <thirdparty/glm/glm/glm.hpp>

#include "Ray.h"

#define MACHINE_EPSILON (std::numeric_limits<float>::epsilon() * 0.5)

namespace Chroma
{
	struct Bounds3
	{
		Bounds3()
		{
			min = glm::vec3(1000000, 1000000, 1000000);
			max = glm::vec3(-1000000, -1000000, -1000000);
		}

		Bounds3(glm::vec3 min_, glm::vec3 max_)
		{
			min = min_;
			max = max_;
		}


		glm::vec3 Diagonal() const;
		float GetSurfaceArea() const;
		float GetVolume() const;

		int MaxExtent() const {
			glm::vec3 d = Diagonal();
			if (d.x > d.y&& d.x > d.z)
				return 0;
			else if (d.y > d.z)
				return 1;
			else
				return 2;
		}

		bool IntersectP(const Ray& ray, float* hitt0, float* hitt1) const
		{
			float t_max = INFINITY;
			float t0 = 0, t1 = t_max;
			for (int i = 0; i < 3; ++i) {
				// Update interval for _i_th bounding box slab
				float inv_ray_dir = 1 / ray.direction[i];
				float t_near = (min[i] - ray.origin[i]) * inv_ray_dir;
				float t_far = (max[i] - ray.origin[i]) * inv_ray_dir;

				// Update parametric interval from slab intersection $t$ values
				if (t_near > t_far) std::swap(t_near, t_far);

				// Update _tFar_ to ensure robust ray--bounds intersection
				t_far *= 1 + 2 * ((3 * MACHINE_EPSILON) / (1 - 3 * MACHINE_EPSILON));
				t0 = t_near > t0 ? t_near : t0;
				t1 = t_far < t1 ? t_far : t1;
				if (t0 > t1) return false;
			}

			if (hitt0)
				*hitt0 = t0;

			if (hitt1)
				*hitt1 = t1;
			return true;
		}

		inline const glm::vec3& operator[](int i) const {
			//DCHECK(i == 0 || i == 1);
			return (i == 0) ? min : max;
		}

		bool IntersectP(const Ray& ray, const glm::vec3& inv_dir,
			const int dirIsNeg[3]) const {
			const Bounds3& bounds = *this;
			// Check for ray intersection against $x$ and $y$ slabs
			float t_min = (bounds[dirIsNeg[0]].x - ray.origin.x) * inv_dir.x;
			float t_max = (bounds[1 - dirIsNeg[0]].x - ray.origin.x) * inv_dir.x;
			float ty_min = (bounds[dirIsNeg[1]].y - ray.origin.y) * inv_dir.y;
			float ty_max = (bounds[1 - dirIsNeg[1]].y - ray.origin.y) * inv_dir.y;

			// Update _tMax_ and _tyMax_ to ensure robust bounds intersection
			t_max *= 1 + 2 * ((3 * MACHINE_EPSILON) / (1 - 3 * MACHINE_EPSILON));
			ty_max *= 1 + 2 * ((3 * MACHINE_EPSILON) / (1 - 3 * MACHINE_EPSILON));
			if (t_min > ty_max || ty_min > t_max) return false;
			if (ty_min > t_min) t_min = ty_min;
			if (ty_max < t_max) t_max = ty_max;

			// Check for ray intersection against $z$ slab
			float tz_min = (bounds[dirIsNeg[2]].z - ray.origin.z) * inv_dir.z;
			float tz_max = (bounds[1 - dirIsNeg[2]].z - ray.origin.z) * inv_dir.z;

			// Update _tzMax_ to ensure robust bounds intersection
			tz_max *= 1 + 2 * ((3 * MACHINE_EPSILON) / (1 - 3 * MACHINE_EPSILON));
			if (t_min > tz_max || tz_min > t_max) return false;
			if (tz_min > t_min) t_min = tz_min;
			if (tz_max < t_max) t_max = tz_max;
			return (t_min < t_max) && (t_max > 0);
		}


		void Extend(const Bounds3& bounds);
		void Extend(const glm::vec3& point);

		glm::vec3 Offset(const glm::vec3& p) const {
			glm::vec3 o = p - min;
			if (max.x > min.x) o.x /= max.x - min.x;
			if (max.y > min.y) o.y /= max.y - min.y;
			if (max.z > min.z) o.z /= max.z - min.z;
			return o;
		}

		static Bounds3 Extend(const Bounds3& b1, const Bounds3& b2);

		glm::vec3 min;
		glm::vec3 max;
	};
}
