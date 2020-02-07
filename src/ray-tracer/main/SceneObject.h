#pragma once

#include <ray-tracer/main/Material.h>
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
	class Mesh
	{
	public:
		Mesh();
		Mesh(std::vector<glm::vec3> m_vertices, std::vector<glm::vec3> m_normals,
			std::vector<glm::vec2> m_texcoords, std::vector<glm::vec3> m_colors, unsigned int face_count);

		inline unsigned int GetFaceCount() const { return m_face_count; }
		inline unsigned int GetVertexCount() const { return m_vertex_count; }

		inline void SetFaceCount(unsigned int f_c) { m_face_count = f_c; }
		inline void SetVertexCount(unsigned int v_c) { m_vertex_count = v_c; }

		inline glm::vec3 GetMaxBound() const { return m_bound_max; }
		inline glm::vec3 GetMinBound() const { return m_bound_min; }

		inline void SetMaxBound(glm::vec3 max_b) { m_bound_max = max_b; }
		inline void SetMinBound(glm::vec3 min_b) { m_bound_min = min_b; }

		/*VECTOR_FIELD_METHODS(m_vertex_positions, Position, Positions,glm::vec3);
		VECTOR_FIELD_METHODS(m_vertex_normals, Normal, Normals, glm::vec3);
		VECTOR_FIELD_METHODS(m_vertex_colors, Color, Colors, glm::vec3);
		VECTOR_FIELD_METHODS(m_vertex_texcoords, TextureCoord, TextureCoords, glm::vec2);
		VECTOR_FIELD_METHODS(m_indices, Index, Indices, int);*/
		/*inline std::vector<glm::vec3> GetVertices() const { return m_vertices; }
		inline std::vector<glm::vec3> GetNormals() const { return m_normals; }
		inline std::vector<glm::vec3> GetColors() const { return m_colors; }
		inline std::vector<glm::vec2> GetTexCoords() const { return m_texcoords; }

		inline void SetVertices(std::vector<glm::vec3> vertices) { m_vertices = vertices; CalculateBounds(); CenterToPivot(); }
		inline void SetNormals(std::vector<glm::vec3> normals) { m_normals = normals; }
		inline void SetColors(std::vector<glm::vec3> colors) { m_colors = colors; }
		inline void SetTexCoords(std::vector<glm::vec2> texcoords) { m_texcoords = texcoords; }*/

		std::vector<glm::vec3> m_vertex_positions;
		std::vector<glm::vec3> m_vertex_normals;
		std::vector<glm::vec3> m_vertex_colors;
		std::vector<glm::vec2> m_vertex_texcoords;

		std::vector<unsigned int> m_indices;

	private:
		void CenterToPivot();
		void CalculateBounds();

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
			glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f));
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
		inline void SetMaterial(Material mat) { m_material = mat; }

		inline Material GetMaterial() { return m_material; }

		void Draw(DrawMode mode);


	private:
		void RecalculateModelMatrix();

		bool m_visible_in_editor = false;
		bool m_visible = true;
		bool m_pickable = true;

		std::string m_name;

		glm::vec3 m_position;
		glm::quat m_rotation;
		glm::vec3 m_scale;

		glm::mat4 m_model_matrix = glm::mat4(1.0);

		Chroma::Texture m_texture;
		Material m_material = Material();

		Mesh m_mesh; //TODO: multiple mesh ?

		Chroma::OpenGLVertexArrayObject m_vao;
		std::vector<std::shared_ptr<Chroma::VertexBuffer>> m_vertex_buffers;
		std::shared_ptr<Chroma::IndexBuffer> m_index_buffer;
	};
}
