#pragma once
#include <ray-tracer/accelerationStructures/AccelerationStructure.h>
#include <ray-tracer/accelerationStructures/BVH.h>
#include <ray-tracer/main/Camera.h>
#include <ray-tracer/main/SceneObject.h>

#include <stdio.h>
#include <map>
#include <vector>


namespace CHR
{
	class Scene
	{
	public:
		Scene(std::string name, Shader* shader);
		~Scene();

		void AddSceneObject(std::string name, std::shared_ptr<SceneObject> object);
		void AddCamera(std::string name, Camera* cam);

		std::shared_ptr<SceneObject> GetSceneObject(std::string name) { return m_scene_objects[name]; }
		inline Camera* GetCamera(std::string name) { return m_cameras[name]; }
		//inline Camera* GetActiveCamera() { return m_cameras[active_cam_name]; }

		void AddLight(std::string name, std::shared_ptr<Light> li);

		void InitBVH(int maxPrimsInNode = 1, SplitMethod splitMethod = (SplitMethod)0);
		inline bool IsAccelerationReady() { return m_accel_structure != NULL; }
		bool Intersect(const Ray ray, IntersectionData* isect_data) const;

		inline std::shared_ptr<Light> GetLight(std::string name) { return m_lights[name]; } //TODO: add null check


		void Render(Camera* cam, DrawMode = DrawMode::TRI);

		glm::vec4 m_sky_color = {0.15f * 0.3, 0.15f * 0.3, 0.2f * 0.3, 1.0f };
		std::shared_ptr<TextureMap> m_sky_texture;
		bool m_map_texture_to_sphere = false;
		glm::vec3 m_ambient_l = { 0.0, 0.0, 0.0f };

		std::map<std::string, std::shared_ptr<SceneObject>> m_scene_objects;
		std::map<std::string, std::shared_ptr<Light>> m_lights;

	private:
		AccelerationStructure* m_accel_structure = nullptr;
		friend class Editor;
		std::string m_name;

		std::map<std::string, Camera*> m_cameras;


		class SceneData
		{
		public:
			SceneData(Shader* shader)
				:m_shader(shader)
			{}

			inline void SetModel(glm::mat4 mat)
			{
				*m_model = mat;
				*m_normal_mat = (glm::transpose(glm::inverse(*m_model)));
				//m_shader.UpdateUniforms();
			}
			inline void SetView(glm::mat4 mat)
			{
				*m_view = mat;
				//m_shader.UpdateUniforms();
			}
			inline void SetProj(glm::mat4 mat)
			{
				*m_proj = mat;
				//m_shader.UpdateUniforms();
			}
			inline void SetMaterial(Material* material)
			{
				*m_material = *material;
			}
			inline void SetCamPos(glm::vec3 cam_pos)
			{
				*m_cam_pos = cam_pos;
			}

			inline glm::mat4* GetModel() { return m_model; }
			inline glm::mat4* GetView() { return m_view; }
			inline glm::mat4* GetProj() { return m_proj; }
			inline glm::mat4* GetNormalMat() { return m_normal_mat; }
			inline Material* GetMaterial() { return m_material; }
			inline glm::vec3* GetCamPos() { return m_cam_pos; }

			Shader* m_shader;
		private:
			glm::mat4* m_model = new glm::mat4(1.0f);
			glm::mat4* m_view = new glm::mat4(1.0f);
			glm::mat4* m_proj = new glm::mat4(1.0f);
			glm::mat4* m_normal_mat = new glm::mat4(1.0f);
			Material* m_material = new Material();
			glm::vec3* m_cam_pos = new glm::vec3(0.0f);

		};
		SceneData* m_scene_data;
	};
}
