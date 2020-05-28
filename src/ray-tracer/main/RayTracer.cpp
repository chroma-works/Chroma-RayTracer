#include "RayTracer.h"

#include <future>
#include <limits>

#include <thirdparty\glm\glm\glm.hpp>
#include <thirdparty\glm\glm\gtx\norm.hpp>
#include <thirdparty\glm\glm\gtx\component_wise.hpp>
#include <iostream>

namespace Chroma
{
	std::atomic<float> progress_pers;
	bool done = false;
	bool run_bar = true;//TODO: FIX ASYNC CALLS
	void PrintProgressBar(std::string tag)
	{
		if (run_bar)
		{
			while (progress_pers <= 9.89999999 && !done) 
			{
				int barWidth = 70;
				int pos = barWidth * progress_pers;

				Sleep(100);

				std::cout << tag + std::string(" [");
				for (int i = 0; i < barWidth; i++) {
					if (i < pos) std::cout << "=";
					else if (i == pos) std::cout << ">";
					else std::cout << " ";
				}
				std::cout << "]" << int(progress_pers * 100.0) << " %\r";
				std::cout.flush();
			}
			std::cout << std::endl;
		}
	}

	bool RayTracer::TestShadow(const Scene& scene, const IntersectionData* isect_data, const std::shared_ptr<Light> li)
	{
		Ray shadow_ray(isect_data->position + isect_data->normal * m_settings.shadow_eps);
		//shadow_ray.jitter_t = ray.jitter_t; // Uncomment if problems occur
		float distance = 0.0f;
		switch (li->m_type)
		{
		case LIGHT_T::point:
			shadow_ray.direction = glm::normalize(dynamic_cast<PointLight*>(li.get())->m_position - isect_data->position);
			distance = glm::distance(isect_data->position, dynamic_cast<PointLight*>(li.get())->m_position);
			break;
		case LIGHT_T::directional:
			shadow_ray.direction = -glm::normalize(dynamic_cast<DirectionalLight*>(li.get())->m_direction);
			distance = INFINITY;
			break;
		case LIGHT_T::spot:
			shadow_ray.direction = glm::normalize(dynamic_cast<SpotLight*>(li.get())->m_position - isect_data->position);
			distance = glm::distance(isect_data->position, dynamic_cast<SpotLight*>(li.get())->m_position);
			break;
		case LIGHT_T::area:
			glm::vec3 u, v;
			Utils::CreateOrthonormBasis(dynamic_cast<AreaLight*>(li.get())->m_normal, u, v);

			thread_local static std::random_device dev;
			std::uniform_real_distribution<float> dist(-0.5, 0.5);
			thread_local static std::mt19937_64 mt(dev());

			glm::vec3 sample_l_pos = dynamic_cast<AreaLight*>(li.get())->m_position +
				(dist(mt) * dynamic_cast<AreaLight*>(li.get())->m_size * u +
				dist(mt) * dynamic_cast<AreaLight*>(li.get())->m_size * v);

			shadow_ray.direction = glm::normalize(sample_l_pos - isect_data->position);
			//shadow_ray.intersect_eps = m_settings.intersection_eps;
			distance = glm::distance(isect_data->position, sample_l_pos);
			break;
		/*default:
			break;*/
		}

		IntersectionData shadow_data;
		bool shadowed = m_settings.calc_shadows &&
			(scene.m_accel_structure->Intersect(shadow_ray, &shadow_data) &&
			(glm::distance(isect_data->position, shadow_data.position) - distance < 0.0f));
		return shadowed;
	}

	RayTracer::RayTracer()
	{
		m_rendered_image = new Image(m_settings.resolution.x, m_settings.resolution.y, m_settings.ldr_post_process);
		for (int i = 0; i < m_settings.resolution.x; i++)
			for (int j = 0; j < m_settings.resolution.y; j++)
				m_rendered_image->SetPixel(i, j, glm::vec3(0.0f, 0.0f, 0.0f));
		m_rt_worker = &RayTracer::RecursiveTraceWorker;
	}


	void RayTracer::Render(Camera* cam, Scene& scene, bool print_progress)
	{
		if (!scene.m_accel_structure)
		{
			return;
		}

		progress_pers = 0.0f;
		done = false;
		job_index = { 0 };
		run_bar = print_progress;
		auto future_function = async(std::launch::async, PrintProgressBar, "Rendering");

		std::thread** threads = new std::thread * [m_settings.thread_count];

		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

		for (int i = 0; i < m_settings.thread_count; i++)
			threads[i] = new std::thread(m_rt_worker, this, cam, std::ref(scene), i);

		for (int i = 0; i < m_settings.thread_count; i++)
		{
			threads[i]->join();
			delete threads[i];
		}

		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		delete[] threads;

		std::chrono::duration<float> fs = end - start;
		std::chrono::milliseconds d = std::chrono::duration_cast<std::chrono::milliseconds>(fs);


		unsigned int triangle_count = 0;

		for (auto it = scene.m_scene_objects.begin(); it != scene.m_scene_objects.end(); it++)
		{
			if(it->second->GetShapeType() != SHAPE_T::sphere)
				triangle_count += it->second->m_mesh->GetFaceCount();

			glm::vec3 min = it->second->m_mesh->GetMinBound();
			glm::vec3 max = it->second->m_mesh->GetMaxBound();
		}
		if (m_rendered_image->IsHDR())
		{
			if (m_settings.ldr_post_process == 1)
				m_rendered_image->ToneMap(cam->m_key_val, cam->m_burn_perc,
					cam->m_saturation, cam->m_gamma);
			else
				m_rendered_image->Clamp(0, 255);
		}
		//while (progress_pers != 1.0f)
		//	if (progress_pers == 1.0f)
		//		break;
		done = true;
		if (print_progress)
		{
			Sleep(200);
			CH_TRACE("Render info:\n\tTriangles :" + std::to_string(triangle_count) +
				"\n\tResolution: (" + std::to_string(cam->GetResolution().x) + ", " + std::to_string(cam->GetResolution().y)
				+")\n\tSample per pixel: " + std::to_string(cam->GetNumberOfSamples()) + 
				"\n\tRendered in " + std::to_string(fs.count()) + "s" 
				+ "\n\tThreads: " + std::to_string(m_settings.thread_count));
		}
	}

	void RayTracer::RayCastWorker(Camera* cam, Scene& scene, int idx)
	{
		//glm::vec3 cam_pos = cam->GetPosition();
		//glm::vec2 top_left = cam->GetNearPlane()[0];
		//glm::vec2 bottom_right = cam->GetNearPlane()[1];
		//float dist = cam->GetNearDist();

		//glm::vec3 up = glm::normalize(cam->GetUp());
		//glm::vec3 forward = glm::normalize(cam->GetGaze());
		//glm::vec3 down = -up;
		//glm::vec3 right = glm::normalize(glm::cross(forward, up));
		//glm::vec3 left = -right;

		//const glm::vec3 top_left_w = cam_pos + forward * dist + up * top_left.y + left * glm::abs(top_left.x);
		//Ray primary_ray(cam_pos);

		//const glm::vec3 right_step = (right)*glm::abs(top_left.x - bottom_right.x) / (float)m_settings.resolution.x;
		//const glm::vec3 down_step = (down)*glm::abs(top_left.y - bottom_right.y) / (float)m_settings.resolution.y;

		//IntersectionData isect_data;
		//IntersectionData shadow_data;

		//int col_start = (float)idx / (float)m_settings.thread_count * m_settings.resolution.x;
		//int col_end = idx == m_settings.thread_count -1 ? m_settings.resolution.x :
		//	(float)(idx + 1) / (float)m_settings.thread_count * m_settings.resolution.x;

		//for (int i = col_start; i < col_end; i++)
		//{
		//	for (int j = 0; j < m_settings.resolution.y; j++)
		//	{
		//		glm::vec3 color = scene.m_sky_color;

		//		primary_ray.direction = glm::normalize(top_left_w + right_step * (i + 0.5f) + down_step * (j + 0.5f) - primary_ray.origin);
		//		//Go over scene objects and lights
		//		if (scene.m_accel_structure->Intersect(primary_ray, &isect_data))//Hit
		//		{
		//			color = { 0,0,0 };
		//			//lighting calculation
		//			std::map<std::string, std::shared_ptr<PointLight>>::iterator it2;
		//			for (it2 = scene.m_point_lights.begin(); it2 != scene.m_point_lights.end(); it2++)
		//			{
		//				std::shared_ptr<PointLight> pl = it2->second;
		//				glm::vec3 e_vec = glm::normalize(primary_ray.origin - isect_data.position);
		//				glm::vec3 l_vec = glm::normalize(pl->position - isect_data.position);

		//				//Shadow calculation	
		//				bool shadowed = false;
		//				Ray shadow_ray(isect_data.position + isect_data.normal * m_settings.shadow_eps);
		//				shadow_ray.direction = glm::normalize(pl->position - shadow_ray.origin);
		//					
		//				shadowed = m_settings.calc_shadows && (scene.m_accel_structure->Intersect(shadow_ray, &shadow_data) &&
		//					shadow_data.t < glm::distance(isect_data.position, pl->position));

		//				if (!shadowed)
		//				{
		//					float d = glm::distance(pl->position, isect_data.position);

		//					//Kd * I * cos(theta) /d^2 
		//					glm::vec3 diffuse = isect_data.material->m_diffuse * pl->intensity *
		//						glm::max(glm::dot(isect_data.normal, l_vec), 0.0f) / (glm::length(isect_data.normal) * glm::length(l_vec)) / (d * d);
		//					//Ks* I * max(0, h . n)^s / d^2
		//					glm::vec3 h = glm::normalize((e_vec + l_vec) / glm::length(e_vec + l_vec));
		//					glm::vec3 specular = isect_data.material->m_specular * pl->intensity *
		//						glm::pow(glm::max(0.0f, glm::dot(h, glm::normalize(isect_data.normal))), isect_data.material->m_shininess) / (d * d);
		//					color += specular + diffuse;
		//				}
		//			}
		//			//Ka * Ia
		//			glm::vec3 ambient = scene.m_ambient_l * isect_data.material->m_ambient;
		//			color += ambient;
		//		}
		//		m_rendered_image->SetPixel(i, j, color);
		//	}

		//	progress_pers = progress_pers + (1.0f) / ((float)(m_settings.resolution.x));
		//	/*if(idx == m_settings.thread_count - 1)
		//		CH_TRACE(std::to_string(progress_pers * 100.0f) + std::string("% complete"));*/
		//}
	}

	int tile_size = 8;
	void RayTracer::RecursiveTraceWorker(Camera* cam, Scene& scene, int thread_idx)
	{
		glm::vec3 cam_pos = cam->GetPosition();
		glm::vec2 top_left = cam->GetNearPlane()[0];
		glm::vec2 bottom_right = cam->GetNearPlane()[1];
		float dist = cam->GetNearDist();

		glm::vec3 up = glm::normalize(cam->GetUp());
		glm::vec3 forward = glm::normalize(cam->GetGaze());
		glm::vec3 down = -up;
		glm::vec3 right = cam->m_left_handed ? -glm::normalize(glm::cross(forward, up)) : 
			glm::normalize(glm::cross(forward, up));
		glm::vec3 left = -right;

		const glm::vec3 top_left_w = cam_pos + forward * dist + up * top_left.y + left * glm::abs(top_left.x);
		Ray primary_ray(cam_pos);

		const glm::vec3 right_step = (right)*glm::abs(top_left.x - bottom_right.x) / (float)m_settings.resolution.x;
		const glm::vec3 down_step = (down)*glm::abs(top_left.y - bottom_right.y) / (float)m_settings.resolution.y;

		IntersectionData intersection_data;
		IntersectionData shadow_data;

		const int tile_count_x = (m_settings.resolution.x + tile_size - 1) / tile_size;
		const int tile_count_y = (m_settings.resolution.y + tile_size - 1) / tile_size;
		const int max_job_index = tile_count_x * tile_count_y;

		int idx = thread_idx;


		while (idx < max_job_index)
		{
			glm::ivec2 rect_min = glm::ivec2((idx % tile_count_x) * tile_size, (idx / tile_count_x) * tile_size);
			glm::ivec2 rect_max = rect_min + glm::ivec2(tile_size, tile_size);

			rect_max = (glm::min)(rect_max, m_settings.resolution);

			for (int j = rect_min.y; j < rect_max.y; j++)
			{
				for (int i = rect_min.x; i < rect_max.x; i++)
				{
					glm::vec3 color = scene.m_sky_color;
					for (int n = 0; n < cam->GetNumberOfSamples(); n++)
					{
						auto offset = Utils::SampleUnitSquare();
						auto lens_offset = Utils::SampleUnitDisk();
						//DoF Lens calculation
						glm::vec3 lens_point = cam_pos +
							cam->GetApertureSize() * (lens_offset.x * right + lens_offset.y * up);
						glm::vec3 pixel_point = top_left_w +
							right_step * (i + offset.x)
							+ down_step * (j + offset.y);
						glm::vec3 dir = glm::normalize(pixel_point - cam_pos);
						glm::vec3 focal_point = cam_pos +
							cam->GetFocalDistance() / glm::dot(dir, cam->GetGaze()) * dir;

						primary_ray.origin = lens_point;
						primary_ray.direction = glm::normalize(focal_point - primary_ray.origin);
						primary_ray.jitter_t = Utils::RandFloat();

						if(cam->GetNumberOfSamples() ==1)
							primary_ray.direction = glm::normalize(top_left_w + right_step * (i + 0.5f) + down_step * (j + 0.5f) - primary_ray.origin);

						glm::vec3 sample_color = RecursiveTrace(primary_ray, scene, 0, {i,j});
						color += sample_color / (float)cam->GetNumberOfSamples();//Box Filter
					}
					m_rendered_image->SetPixel(i, j, color);
					progress_pers = progress_pers + (1.0f) / ((float)(glm::compMul(m_settings.resolution)));
				}


				/*if (idx == m_settings.thread_count - 1)
					CH_TRACE(std::to_string(progress_pers * 100.0f) + std::string("% complete"));*/
			}
			idx = job_index++;
		}
	}

	glm::vec3 RayTracer::RecursiveTrace(const Ray& ray, Scene& scene, int depth, glm::ivec2 pixel_cood)
	{
		IntersectionData isect_data;
		scene.m_accel_structure->Intersect(ray, &isect_data);

		glm::vec3 color = { 0,0,0 };
		bool inside = false;

		if (!isect_data.hit)
		{
			if (scene.m_sky_texture)
			{
				if (scene.m_map_texture_to_sphere)
				{
					auto dir = glm::normalize(ray.direction);
					float u = 0.5f - atan2(dir.z, dir.x) * (0.5f / Utils::PI);
					float v = acosf(dir.y)  / Utils::PI;
					auto t_coord = glm::vec3(u, v, NAN);
					return scene.m_sky_texture->SampleAt(t_coord)*255.0f;
				}
				else
					return scene.m_sky_texture->SampleAt({ pixel_cood.x / (float)m_settings.resolution.x,
						pixel_cood.y / (float)m_settings.resolution.y, 0 }) * 255.0f;
			}
			else
				return scene.m_sky_color;
		}
		else if (m_settings.calc_reflections && 
			isect_data.material->type == MAT_TYPE::mirror && depth < m_settings.recur_depth) 
		{
			// compute reflection
			Ray reflection_ray(isect_data.position + isect_data.normal * m_settings.shadow_eps);

			//For glossy objects
			glm::vec3 r = glm::normalize(glm::reflect(ray.direction, isect_data.normal));
			glm::vec3 u, v;
			Utils::CreateOrthonormBasis(r, u, v);
			reflection_ray.direction = glm::normalize(r + isect_data.material->m_roughness * 
				(Utils::RandFloat(-0.5, 0.5) * u + Utils::RandFloat(-0.5, 0.5) * v));
			reflection_ray.intersect_eps = m_settings.intersection_eps;
			reflection_ray.jitter_t = Utils::RandFloat();

			glm::vec3 reflection_color = RecursiveTrace(reflection_ray, scene, depth + 1, pixel_cood) * ((Mirror*)(isect_data.material))->m_mirror_reflec;
			color += reflection_color;
		}
		else if (m_settings.calc_reflections &&
			isect_data.material->type == MAT_TYPE::conductor && depth < m_settings.recur_depth)
		{
			// compute reflection
			Ray reflection_ray(isect_data.position + isect_data.normal * m_settings.shadow_eps);
			//For glossy objects
			glm::vec3 r = glm::normalize(glm::reflect(ray.direction, isect_data.normal));
			glm::vec3 u, v;
			Utils::CreateOrthonormBasis(r, u, v);

			reflection_ray.direction = glm::normalize(r + isect_data.material->m_roughness *
				(Utils::RandFloat(-0.5, 0.5) * u + Utils::RandFloat(-0.5, 0.5) * v));
			reflection_ray.intersect_eps = m_settings.intersection_eps;
			reflection_ray.jitter_t = Utils::RandFloat();

			float cos_theta = glm::dot(-ray.direction, isect_data.normal);

			glm::vec3 reflection_color = RecursiveTrace(reflection_ray, scene, depth + 1, pixel_cood) * ((Conductor*)isect_data.material)->GetFr(cos_theta) *
				((Conductor*)(isect_data.material))->m_mirror_reflec;
			color += reflection_color;
		}
		else if (isect_data.material->type == MAT_TYPE::dielectric && depth < m_settings.recur_depth)
		{
			float cos_i = glm::dot(ray.direction, isect_data.normal);
			float ni = 1.0f;
			float nt = ((Dielectric*)(isect_data.material))->m_refraction_ind;

			glm::vec3 proper_normal = isect_data.normal;

			if (inside = (cos_i > 0.0f))
			{
				std::swap(ni, nt);
				proper_normal = -isect_data.normal;
			}

			float fr = ((Dielectric*)isect_data.material)->GetFr(cos_i);
			cos_i = std::abs(cos_i);
			glm::vec3 reflection_color = { 0,0,0 };
			if (m_settings.calc_reflections)
			{
				Ray reflection_ray(isect_data.position + proper_normal * m_settings.shadow_eps);
				//For glossy objects
				glm::vec3 r = glm::normalize(glm::reflect(ray.direction, isect_data.normal));
				glm::vec3 u, v;
				Utils::CreateOrthonormBasis(r, u, v);

				reflection_ray.direction = glm::normalize(r + isect_data.material->m_roughness *
					(Utils::RandFloat(-0.5, 0.5) * u + Utils::RandFloat(-0.5, 0.5) * v));
				reflection_ray.intersect_eps = m_settings.intersection_eps;
				reflection_ray.jitter_t = Utils::RandFloat();

				reflection_color = RecursiveTrace(reflection_ray, scene, depth + 1, pixel_cood) * fr;
			}


			glm::vec3 refraction_color = { 0,0,0 };
			if (fr < 1.0f && m_settings.calc_refractions)
			{
				Ray refraction_ray(isect_data.position - proper_normal * m_settings.shadow_eps);
				refraction_ray.direction = glm::normalize(glm::refract(ray.direction, proper_normal, ni / nt));
				refraction_ray.intersect_eps = m_settings.intersection_eps;
				refraction_ray.jitter_t = Utils::RandFloat();

				refraction_color = RecursiveTrace(refraction_ray, scene, depth + 1, pixel_cood) * (1.0f - fr);
			}

			color += (reflection_color + refraction_color);
			if (inside)
			{
				glm::vec3 absorbance = -((Dielectric*)(isect_data.material))->m_absorption_coeff *
					glm::distance(ray.origin, isect_data.position) * 1.0f;
				color *= exp(absorbance);
			}
		}
		// point is illuminated
		if (isect_data.hit && !inside)
		{
			bool replace_all = ((isect_data.tex_map) &&
				(isect_data.tex_map[0].GetDecalMode() == DECAL_M::re_all));
			//Ka * Ia
			if(!replace_all)
			{
				glm::vec3 ambient = scene.m_ambient_l * isect_data.material->m_ambient;
				color += ambient;
			}

			//lighting calculation
			for (auto it = scene.m_lights.begin(); it != scene.m_lights.end(); it++)
			{
				IntersectionData shadow_data;
				std::shared_ptr<Light> li = it->second;
				glm::vec3 e_vec = glm::normalize(ray.origin - isect_data.position);
				//glm::vec3 l_vec = glm::normalize(pl->position - isect_data.position);

				if (!TestShadow(scene, &isect_data, li) ||
					replace_all)
					color += isect_data.Shade(li, e_vec);
			}
		}
		return color;
	}

	void RayTracer::SetResoultion(const glm::ivec2& resolution)
	{
		if (m_settings.resolution == resolution)
			return;

		m_settings.resolution = resolution;

		delete m_rendered_image;

		m_rendered_image = new Image(m_settings.resolution.x, m_settings.resolution.y, m_settings.ldr_post_process);
		for (int i = 0; i < m_settings.resolution.x; i++)
			for (int j = 0; j < m_settings.resolution.y; j++)
				m_rendered_image->SetPixel(i, j, glm::vec3(0.0f, 0.0f, 0.0f));
	}

	void RayTracer::SetRenderMode(RT_MODE mode)
	{
		switch (mode)
		{
		case Chroma::ray_cast:
			m_rt_worker = &RayTracer::RayCastWorker;
			break;
		case Chroma::recursive_trace:
			m_rt_worker = &RayTracer::RecursiveTraceWorker;
			break;
		case Chroma::size:
			break;
		default:
			break;
		}
	}

}
