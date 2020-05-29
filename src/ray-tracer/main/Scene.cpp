#include "Scene.h"
#include <ray-tracer/accelerationStructures/BVH.h>

namespace CHR
{
	Scene::Scene(std::string name, Shader* shader)
		: m_name(name)
	{
		m_scene_data = new SceneData(shader);

		m_scene_data->m_shader->Bind();
		m_scene_data->m_shader->CreateUniform(Shader::MODEL_SH, ShaderDataType::Mat4, m_scene_data->GetModel());
		m_scene_data->m_shader->CreateUniform(Shader::VIEW_SH, ShaderDataType::Mat4, m_scene_data->GetView());
		m_scene_data->m_shader->CreateUniform(Shader::PROJ_SH, ShaderDataType::Mat4, m_scene_data->GetProj());
		m_scene_data->m_shader->CreateUniform(Shader::NORM_MAT_SH, ShaderDataType::Mat4, m_scene_data->GetNormalMat());
		m_scene_data->m_shader->CreateUniform(Shader::CAM_POS_SH, ShaderDataType::Float3, m_scene_data->GetCamPos());
		m_scene_data->m_shader->CreateUniform((m_scene_data->GetMaterial()));
	}

	Scene::~Scene()
	{
		delete m_scene_data;
		m_cameras.erase(m_cameras.begin(), m_cameras.end());
	}

	void Scene::AddSceneObject(std::string name, std::shared_ptr<SceneObject> object)
	{
		m_scene_objects[name] = object;
	}

	void Scene::AddCamera(std::string name, Camera* cam)
	{
		m_cameras[name] = cam;
	}

	void Scene::AddLight(std::string name, std::shared_ptr<Light> li)
	{
		m_lights[name] = li;
		switch (li->m_type)
		{
		case LIGHT_T::point:
			m_scene_data->m_shader->AddLight(std::static_pointer_cast<PointLight>(li));
			break;
		case LIGHT_T::directional:
			m_scene_data->m_shader->AddLight(std::static_pointer_cast<DirectionalLight>(li));
			break;
		case LIGHT_T::spot:
			m_scene_data->m_shader->AddLight(std::static_pointer_cast<SpotLight>(li));
		default:
			break;
		}
	}

	void Scene::InitBVH(int maxPrimsInNode, SplitMethod splitMethod)
	{
		if (m_accel_structure)
			delete m_accel_structure;

		m_accel_structure = new BVH(*this, maxPrimsInNode, splitMethod);
	}


	void Scene::Render(Camera* cam, DrawMode mode)
	{
		if (!cam)
			cam = m_cameras.begin()->second;
		if (!cam)
			CH_FATAL("Failed to render no valid Camera Provided!");

		m_scene_data->SetView(cam->GetViewMatrix());
		m_scene_data->SetProj(cam->GetProjectionMatrix());
		m_scene_data->SetCamPos(cam->GetPosition());
		for (std::pair<std::string, std::shared_ptr<SceneObject>> element : m_scene_objects)
		{
			std::shared_ptr<SceneObject> scn_obj = element.second;
			if (scn_obj->IsVisibleInEditor())
			{
				m_scene_data->SetModel(scn_obj->GetModelMatrix());

				Material* mat = scn_obj->GetMaterial().get();
				if (mat->type == MAT_TYPE::conductor)
				{
					glm::vec3 color = ((Conductor*)mat)->m_mirror_reflec;
					mat = new Material();
					mat->m_ambient += color;
					mat->m_diffuse += color;
					mat->m_specular += glm::vec3({ 1,1,1 });

					mat->m_ambient = glm::normalize(mat->m_ambient);
					mat->m_diffuse = glm::normalize(mat->m_diffuse);
					mat->m_specular = glm::normalize(mat->m_specular);
				}
				else if (mat->type == MAT_TYPE::dielectric)
				{
					glm::vec3 color = glm::normalize(10.0f * (glm::vec3(1,1,1) - ((Dielectric*)mat)->m_absorption_coeff));
					mat = new Material();
					mat->m_diffuse += color;
					mat->m_specular += color;

					mat->m_ambient = glm::normalize(mat->m_ambient);
					mat->m_diffuse = glm::normalize(mat->m_diffuse);
					mat->m_specular = glm::normalize(mat->m_specular);
				}
				else if (mat->type == MAT_TYPE::mirror)
				{
					glm::vec3 color = ((Mirror*)mat)->m_mirror_reflec;
					mat = new Material();
					mat->m_diffuse += color;
					mat->m_specular += color;

					mat->m_ambient = glm::normalize(mat->m_ambient);
					mat->m_diffuse = glm::normalize(mat->m_diffuse);
					mat->m_specular = glm::normalize(mat->m_specular);
				}

				m_scene_data->SetMaterial(mat);

				m_scene_data->m_shader->UpdateUniforms();
				m_scene_data->m_shader->Bind();
				scn_obj->Draw(mode);
			}
		}
	}
}