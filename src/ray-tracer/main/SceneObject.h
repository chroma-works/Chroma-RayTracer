#pragma once

#include <ray-tracer/main/Material.h>
#include <ray-tracer/main/Ray.h>
#include <ray-tracer/editor/Texture.h>
#include <ray-tracer/editor/Shader.h>
#include <ray-tracer/editor/Buffer.h>
#include <ray-tracer/openGL/OpenGLBuffer.h>
#include <ray-tracer/openGL/OpenGLVertexArrayObject.h>

#include <stdio.h>
#include <string>
#include <thirdparty/glm/glm/glm.hpp>
#include <thirdparty/glm/glm/gtc/quaternion.hpp>
#include <thirdparty/glm/glm/gtx/euler_angles.hpp>


namespace Chroma
{
#define VECTOR_FIELD_METHODS(VAR, NAME_S, NAME_P, TYPE) std::vector<TYPE> Get##NAME_P() const { return VAR; }\
                                                    void Set##NAME_P(std::vector<TYPE> NAME) { VAR = NAME;}\
                                                    void AddTo##NAME_P(TYPE val) { VAR.push_back(val);}\
                                                    void ResizeSpaceFrom##NAME_P(unsigned int size) {VAR.resize(size);}\
                                                    void Set##NAME_S##At(unsigned int index, TYPE val) {VAR.at(index) = val;}
	enum class RT_INTR_TYPE {mesh, triangle, sphere};

	class Mesh
	{
	public:
		Mesh();
		Mesh(std::vector<glm::vec3> m_vertices, std::vector<glm::vec3> m_normals,
			std::vector<glm::vec2> m_texcoords, std::vector<glm::vec3> m_colors, unsigned int face_count, bool cntr_piv = false);
		Mesh(std::vector<glm::vec3> m_vertices, std::vector<glm::vec3> m_normals,
			std::vector<glm::vec2> m_texcoords, std::vector<glm::vec3> m_colors, std::vector<unsigned int> m_indices, bool cntr_piv = false);


		inline unsigned int GetFaceCount() const { return m_face_count; }
		inline unsigned int GetVertexCount() const { return m_vertex_count; }

		inline void SetFaceCount(unsigned int f_c) { m_face_count = f_c; }
		inline void SetVertexCount(unsigned int v_c) { m_vertex_count = v_c; }

		inline glm::vec3 GetMaxBound() const { return m_bound_max; }
		inline glm::vec3 GetMinBound() const { return m_bound_min; }

		inline void SetMaxBound(glm::vec3 max_b) { m_bound_max = max_b; }
		inline void SetMinBound(glm::vec3 min_b) { m_bound_min = min_b; }

		std::vector<glm::vec3> m_vertex_positions;
		std::vector<glm::vec3> m_vertex_normals;
		std::vector<glm::vec3> m_vertex_colors;
		std::vector<glm::vec2> m_vertex_texcoords;

		std::vector<unsigned int> m_indices;

	private:
		void CenterToPivot();
		void CalculateBounds();
		void OrderVerticesCCW();

		unsigned int m_face_count = 0;
		unsigned int m_vertex_count = 0;

		glm::vec3 m_bound_max;
		glm::vec3 m_bound_min;
	};

	enum DrawMode {
		TRI = 0x0004, TRI_STRIP = 0x0005,
		TRI_FAN = 0x0006, LIN = 0x1B01, LIN_LOOP = 0x0002, LIN_STRIP = 0x0003
	};

	class SceneObject
	{
	public:
		SceneObject(Mesh mesh, std::string name,
			glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), RT_INTR_TYPE rt_intersect = RT_INTR_TYPE::mesh);
		//~SceneObject();


		inline void HideInEditor(bool hide) { m_visible_in_editor = hide; }
		inline void SetVisible(bool visible) { m_visible = visible; }
		inline void SetPickable(bool pickable) { m_pickable = pickable; }

		inline bool IsVisibleInEditor() const { return m_visible_in_editor; }
		inline bool IsVisible() const { return m_visible; }
		inline bool IsPickable() const { return m_pickable; }


		inline void SetPosition(const glm::vec3 pos) { m_position = pos; RecalculateModelMatrix(); }
		inline void SetRotation(const glm::quat rot) { m_rotation = rot; RecalculateModelMatrix(); }
		inline void SetScale(const glm::vec3 sca) { m_scale = sca; RecalculateModelMatrix(); }

		inline glm::mat4 GetModelMatrix() const { return m_model_matrix; }
		inline glm::vec3 GetPosition() const { return m_position; }
		inline glm::quat GetRotation() const { return m_rotation; }
		inline glm::vec3 GetScale() const { return m_scale; }

		inline void Translate(const glm::vec3 vec) { m_position += vec; RecalculateModelMatrix(); }
		inline void RotateAngleAxis(const float angle, const glm::vec3 axis) {
			m_rotation = glm::angleAxis(angle, axis) * m_rotation;
			RecalculateModelMatrix();
		}
		inline void RotateEuler(const glm::vec3 euler_angles) {
			m_rotation = glm::quat(euler_angles) * m_rotation;
			RecalculateModelMatrix();
		}
		inline void Scale(const glm::vec3 scale) { m_scale *= scale; RecalculateModelMatrix(); }
		inline void ResetTransforms() { m_model_matrix = glm::mat4(1.0f); }


		inline void SetTexture(Chroma::Texture tex) { m_texture = tex; }
		inline void SetMaterial(Material mat) { *m_material = mat; }

		inline Material* GetMaterial() const { return m_material; }

		inline RT_INTR_TYPE GetRTIntersectionMethod() { return m_method; }

		inline std::string GetName() const { return m_name; }
		inline void SetName(std::string n) { m_name = n; }

		inline void SetRadius(float radius)
		{
			m_radius = radius;

			m_mesh.SetMinBound(m_position - glm::vec3(radius, radius, radius));
			m_mesh.SetMaxBound(m_position + glm::vec3(radius, radius, radius));
		}

		bool Intersect(Ray ray, float intersection_eps, IntersectionData* intersection_data) const;

		inline bool IntersectBBox(Ray ray, float intersection_eps, float& tHit)
		{
			float tmin, tmax, tymin, tymax, tzmin, tzmax;
			glm::vec3 bounds[2] = { m_mesh.GetMinBound(),  m_mesh.GetMaxBound() };
			bool sign[3] = {(ray.direction.x < 0.0f), (ray.direction.y < 0.0f), (ray.direction.z < 0.0f)  };
			glm::vec3 inv_dir = 1.0f / ray.direction;

			tmin = (bounds[sign[0]].x - ray.origin.x) * inv_dir.x;
			tmax = (bounds[1 - sign[0]].x - ray.origin.x) * inv_dir.x;
			tymin = (bounds[sign[1]].y - ray.origin.y) * inv_dir.y;
			tymax = (bounds[1 - sign[1]].y - ray.origin.y) * inv_dir.y;

			if ((tmin > tymax) || (tymin > tmax))
				return false;

			if (tymin > tmin)
				tmin = tymin;
			if (tymax < tmax)
				tmax = tymax;

			tzmin = (bounds[sign[2]].z - ray.origin.z) * inv_dir.z;
			tzmax = (bounds[1 - sign[2]].z - ray.origin.z) * inv_dir.z;

			if ((tmin > tzmax) || (tzmin > tmax))
				return false;

			if (tzmin > tmin)
				tmin = tzmin;
			if (tzmax < tmax)
				tmax = tzmax;

			tHit = tmin;

			return true;
		}

		void Draw(DrawMode mode);

		float m_radius;
		Mesh m_mesh; //TODO: multiple mesh ?

	private:
		void RecalculateModelMatrix();

		bool m_visible_in_editor = true;
		bool m_visible = true;
		bool m_pickable = true;

		std::string m_name;

		glm::vec3 m_position;
		glm::quat m_rotation;
		glm::vec3 m_scale;

		glm::mat4 m_model_matrix = glm::mat4(1.0);

		Chroma::Texture m_texture;
		Material* m_material;

		bool(SceneObject::* m_intersection_method)(Ray ray, float intersect_eps, IntersectionData* data) const;

		bool IntersectSphere(Ray ray, float intersect_eps, IntersectionData* data) const;
		bool IntersectTriangle(Ray ray, float intersect_eps, IntersectionData* data) const;
		bool IntersectMesh(Ray ray, float intersect_eps, IntersectionData* data) const;

		RT_INTR_TYPE m_method;

		Chroma::OpenGLVertexArrayObject m_vao;
		std::vector<std::shared_ptr<Chroma::VertexBuffer>> m_vertex_buffers;
		std::shared_ptr<Chroma::IndexBuffer> m_index_buffer;
	};
}

