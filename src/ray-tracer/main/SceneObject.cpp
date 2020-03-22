#include "SceneObject.h"

#include <thirdparty/glad/include/glad/glad.h>
#include <thirdparty/glm/glm/gtc/matrix_transform.hpp>
#include <thirdparty/glm/glm/gtx/quaternion.hpp>
#include <thirdparty/glm/glm/gtx/string_cast.hpp>
#include <ray-tracer/editor/AssetImporter.h>

#define MK_SHRD(X) std::make_shared<glm::vec3>(X)

namespace Chroma
{
	Mesh::Mesh()
	{
		m_bound_min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
		m_bound_max = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
	}

	Mesh::Mesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> texcoords, std::vector<glm::vec3> colors, unsigned int face_count, bool cntr_piv)
	{
		m_vertex_positions = vertices;
		m_vertex_normals = normals;
		m_vertex_colors = colors;
		m_vertex_texcoords = texcoords;

		m_vertex_count = vertices.size();
		m_face_count = face_count;

		m_bound_min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
		m_bound_max = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };

		CalculateBounds();
		
		if(cntr_piv)
			CenterToPivot();
	}

	Mesh::Mesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> texcoords, std::vector<glm::vec3> colors, std::vector<unsigned int> indices, bool cntr_piv)
		:Mesh(vertices, normals, texcoords, colors, indices.size() / 3, cntr_piv)
	{
		m_indices = indices;
	}

	void Mesh::OrderVerticesCCW()
	{
		for (int i = 0; i < m_indices.size(); i+=3)
		{
			glm::mat3 mat(m_vertex_positions[m_indices[i]], m_vertex_positions[m_indices[i + 1]], m_vertex_positions[m_indices[i + 2]]);
			if (glm::determinant(mat) < 0)
			{
				//CH_INFO("CW traingle swapped to be CCW");
				std::swap(m_indices[i], m_indices[i + 2]);
			}
		}
	}

	void Mesh::CenterToPivot()
	{
		glm::vec3 center(0, 0, 0);

		for (int i = 0; i < m_vertex_count; i++)
			center += m_vertex_positions[i] / (1.0f * m_vertex_count);

		for (int i = 0; i < m_vertex_count; i++)
			m_vertex_positions[i] -= center;

		m_bound_max -= center;
		m_bound_min -= center;
	}

	void Mesh::CalculateBounds()
	{
		if (m_vertex_count == 0)
		{
			m_bound_min = glm::vec3(0);
			m_bound_max = glm::vec3(0);
		}
		else
		{
			m_bound_min = m_vertex_positions[0];
			m_bound_max = m_vertex_positions[0];

			for (int i = 0; i < m_vertex_count; i++)
			{
				m_bound_min = glm::min(m_vertex_positions[i], m_bound_min);
				m_bound_max = glm::max(m_vertex_positions[i], m_bound_max);
			}
		}
	}


	SceneObject::SceneObject(std::shared_ptr<Mesh> mesh, std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, SHAPE_T t)
		: m_mesh(mesh), m_name(name), m_position(pos), m_rotation(rot), m_scale(scale), m_shape_t(t)
	{
		m_texture = Chroma::Texture("../../assets/textures/white.png");//Set texture to white to avoid all black shaded objects
		m_material = new Material();

		if (m_shape_t == SHAPE_T::sphere)
		{
			if (m_mesh->GetVertexCount() != 0)
			{
				CH_WARN("Using sphere.obj file instead of provided mesh");
			}
			m_mesh = std::make_shared<Mesh>(*AssetImporter::LoadMeshFromOBJ("../../assets/models/sphere.obj"));

			Sphere sphere = Sphere(GetMaterial(), IsVisible());
			//sphere.m_center = GetPosition();
			sphere.m_transform = m_model_matrix;

			m_mesh->m_shapes.push_back(std::make_shared<Sphere>(sphere));
		}
		else
		{
			for (int j = 0; j < mesh->m_indices.size(); j += 3)
			{

				Triangle tri = Triangle(GetMaterial(), IsVisible());

				tri.m_vertices[0] = MK_SHRD(mesh->m_vertex_positions[mesh->m_indices[j]]);
				tri.m_vertices[1] = MK_SHRD(mesh->m_vertex_positions[mesh->m_indices[j + 1]]);
				tri.m_vertices[2] = MK_SHRD(mesh->m_vertex_positions[mesh->m_indices[j + 2]]);

				tri.m_normals[0] = MK_SHRD(glm::normalize(mesh->m_vertex_normals[mesh->m_indices[j]]));
				tri.m_normals[1] = MK_SHRD(glm::normalize(mesh->m_vertex_normals[mesh->m_indices[j + 1]]));
				tri.m_normals[2] = MK_SHRD(glm::normalize(mesh->m_vertex_normals[mesh->m_indices[j + 2]]));

				tri.m_transform = m_model_matrix;

				//if (mesh->uvs.size() > 0)
				//{
				//    shape.uvs[0] = mesh->uvs[j + 0];
				//    shape.uvs[1] = mesh->uvs[j + 1];
				//    shape.uvs[2] = mesh->uvs[j + 2];
				//}

				m_mesh->m_shapes.push_back(std::make_shared<Triangle>(tri));
			}
		}
		//For editor preview render
		InitOpenGLBuffers();
	}

	SceneObject::SceneObject(Mesh* mesh, std::string name, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, SHAPE_T t)
	{
		SceneObject(std::make_shared<Mesh>(*mesh), name, pos, rot, scale, t);
	}

	void SceneObject::Draw(DrawMode mode)
	{
		m_texture.Bind();
		m_vao.Bind();
		glDrawElements(mode, m_index_buffer->GetSize(), GL_UNSIGNED_INT, NULL);
	}

	void SceneObject::RecalculateModelMatrix()
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_position);
		glm::mat4 rotation = glm::eulerAngleYXZ(m_rotation.y, m_rotation.x, m_rotation.z);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_scale);

		//scale[3][3] = 1.0f;

		*m_model_matrix = translation * rotation * scale;
	}
	void SceneObject::InitOpenGLBuffers()
	{
		//Vertex positions buffer
		std::shared_ptr<Chroma::OpenGLVertexBuffer> position_buffer = std::make_shared<Chroma::OpenGLVertexBuffer>((void*)m_mesh->m_vertex_positions.data(),
			m_mesh->m_vertex_positions.size() * sizeof(GLfloat) * 3);

		Chroma::VertexAttribute layout_attribute("in_Position", Chroma::Shader::POS_LAY, Chroma::ShaderDataType::Float3, GL_FALSE);
		Chroma::VertexBufferLayout vertex_buffer_layout;
		vertex_buffer_layout.PushAttribute(layout_attribute);
		position_buffer->SetBufferLayout(vertex_buffer_layout);

		m_vertex_buffers.push_back(position_buffer);

		//Vertex normals buffer
		std::shared_ptr<Chroma::OpenGLVertexBuffer> normal_buffer = std::make_shared<Chroma::OpenGLVertexBuffer>((void*)m_mesh->m_vertex_normals.data(),
			m_mesh->m_vertex_normals.size() * sizeof(GLfloat) * 3);

		Chroma::VertexAttribute layout_attribute2("in_Normal", Chroma::Shader::NORM_LAY, Chroma::ShaderDataType::Float3, GL_FALSE);
		Chroma::VertexBufferLayout vertex_buffer_layout2;
		vertex_buffer_layout2.PushAttribute(layout_attribute2);
		normal_buffer->SetBufferLayout(vertex_buffer_layout2);

		m_vertex_buffers.push_back(normal_buffer);

		//Vertex texture coords buffer
		std::shared_ptr<Chroma::OpenGLVertexBuffer> tex_coord_buffer = std::make_shared<Chroma::OpenGLVertexBuffer>((void*)m_mesh->m_vertex_texcoords.data(),
			m_mesh->m_vertex_texcoords.size() * sizeof(GLfloat) * 2);

		Chroma::VertexAttribute layout_attribute3("in_TexCoord", Chroma::Shader::TEXC_LAY, Chroma::ShaderDataType::Float2, GL_FALSE);
		Chroma::VertexBufferLayout vertex_buffer_layout3;
		vertex_buffer_layout3.PushAttribute(layout_attribute3);
		tex_coord_buffer->SetBufferLayout(vertex_buffer_layout3);

		m_vertex_buffers.push_back(tex_coord_buffer);

		//index buffer
		std::shared_ptr<Chroma::OpenGLIndexBuffer> index_buffer = std::make_shared<Chroma::OpenGLIndexBuffer>(m_mesh->m_indices.data(), m_mesh->m_indices.size());
		m_index_buffer = index_buffer;

		//vertex array object
		m_vao.AddVertexBuffer(position_buffer);
		m_vao.AddVertexBuffer(normal_buffer);
		m_vao.AddVertexBuffer(tex_coord_buffer);
		m_vao.SetIndexBuffer(index_buffer);
	}
}