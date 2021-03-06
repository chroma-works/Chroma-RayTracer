#include "SceneObject.h"

#include <thirdparty/glad/include/glad/glad.h>
#include <thirdparty/glm/glm/gtc/matrix_transform.hpp>
#include <thirdparty/glm/glm/gtx/quaternion.hpp>
#include <thirdparty/glm/glm/gtx/string_cast.hpp>
#include <ray-tracer/editor/AssetImporter.h>
#include <ray-tracer/main/ObjectLight.h>

#define MK_SHRD(X) std::make_shared<glm::vec3>(X)

namespace CHR
{
	Mesh::Mesh()
	{
		m_bound_min = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
		m_bound_max = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() };
	}

	Mesh::Mesh(std::vector<std::shared_ptr<glm::vec3>> vertices, std::vector<std::shared_ptr<glm::vec3>> normals, 
		std::vector<std::shared_ptr<glm::vec2>> texcoords, std::vector<std::shared_ptr<glm::vec3>> colors, unsigned int face_count, bool cntr_piv)
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

	Mesh::Mesh(std::vector<std::shared_ptr<glm::vec3>> vertices, std::vector<std::shared_ptr<glm::vec3>> normals,
		std::vector<std::shared_ptr<glm::vec2>> texcoords, std::vector<std::shared_ptr<glm::vec3>> colors, std::vector<unsigned int> indices, bool cntr_piv)
		:Mesh(vertices, normals, texcoords, colors, indices.size() / 3, cntr_piv)
	{
		m_indices = indices;
	}

	void Mesh::OrderVerticesCCW()
	{
		for (int i = 0; i < m_indices.size(); i+=3)
		{
			glm::mat3 mat(*(m_vertex_positions[m_indices[i]]), 
				*(m_vertex_positions[m_indices[i+1]]),
				*(m_vertex_positions[m_indices[i+2]]));
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
			center += *(m_vertex_positions[i]) / (1.0f * m_vertex_count);

		for (int i = 0; i < m_vertex_count; i++)
			*m_vertex_positions[i] -= center;

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
			m_bound_min = *m_vertex_positions[0];
			m_bound_max = *m_vertex_positions[0];

			for (int i = 0; i < m_vertex_count; i++)
			{
				m_bound_min = glm::min(*m_vertex_positions[i], m_bound_min);
				m_bound_max = glm::max(*m_vertex_positions[i], m_bound_max);
			}
		}
	}
	void Mesh::SmoothNormals()
	{
		for (int i = 0; i < m_indices.size(); i ++)
		{
			*m_vertex_normals[m_indices[i]] = {0,0,0};
		}
		for (int i = 0; i < m_indices.size(); i += 3)
		{
			glm::vec3 a, b;
			a = *m_vertex_positions[m_indices[i + 1]] - *m_vertex_positions[m_indices[i + 0]];
			b = *m_vertex_positions[m_indices[i + 2]] - *m_vertex_positions[m_indices[i + 0]];

			glm::vec3 normal = glm::normalize(glm::cross(a, b));

			*m_vertex_normals[m_indices[i]] += normal;
			*m_vertex_normals[m_indices[i+1]] += normal;
			*m_vertex_normals[m_indices[i+2]] += normal;
		}
		for (int i = 0; i < m_indices.size(); i++)
		{
			*m_vertex_normals[m_indices[i]] = glm::normalize(*m_vertex_normals[m_indices[i]]);
		}
	}


	SceneObject::SceneObject(std::shared_ptr<Mesh> mesh, std::string name, glm::vec3 pos, glm::vec3 rot, 
		glm::vec3 scale, SHAPE_T t, std::vector<unsigned int> tex_inds, glm::vec3 radiance)
		: m_mesh(mesh), m_position(pos), m_rotation(rot), m_scale(scale), m_shape_t(t)
	{
		m_name = name;
		m_texture = CHR::Texture("../../assets/textures/white.png");//Set texture to white to avoid black shaded objects
		m_material = std::shared_ptr<Material>();

		if(m_shape_t != SHAPE_T::sphere)
		{
			for (int j = 0; j < mesh->m_indices.size(); j += 3)
			{

				Triangle tri = Triangle(GetMaterial(), IsVisible());

				tri.m_vertices[0] = (mesh->m_vertex_positions[mesh->m_indices[j]]);
				tri.m_vertices[1] = (mesh->m_vertex_positions[mesh->m_indices[j + 1]]);
				tri.m_vertices[2] = (mesh->m_vertex_positions[mesh->m_indices[j + 2]]);

				tri.m_normals[0] = (mesh->m_vertex_normals[mesh->m_indices[j]]);
				tri.m_normals[1] = (mesh->m_vertex_normals[mesh->m_indices[j + 1]]);
				tri.m_normals[2] = (mesh->m_vertex_normals[mesh->m_indices[j + 2]]);

				if (mesh->m_vertex_texcoords.size() > 0)
				{
					if (tex_inds.size() == 0)
					{
						tri.m_uvs[0] = (mesh->m_vertex_texcoords[mesh->m_indices[j]]);
						tri.m_uvs[1] = (mesh->m_vertex_texcoords[mesh->m_indices[j + 1]]);
						tri.m_uvs[2] = (mesh->m_vertex_texcoords[mesh->m_indices[j + 2]]);
					}
					else
					{
						tri.m_uvs[0] = (mesh->m_vertex_texcoords[tex_inds[j]]);
						tri.m_uvs[1] = (mesh->m_vertex_texcoords[tex_inds[j + 1]]);
						tri.m_uvs[2] = (mesh->m_vertex_texcoords[tex_inds[j + 2]]);
					}
				}


				tri.SetTransform(m_tranform_matrix, m_inverse_tranform_matrix);
				tri.m_visible = IsVisible();

				tri.m_shading_mode = m_mesh->m_shading_mode;

				//if (mesh->uvs.size() > 0)
				//{
				//    shape.uvs[0] = mesh->uvs[j + 0];
				//    shape.uvs[1] = mesh->uvs[j + 1];
				//    shape.uvs[2] = mesh->uvs[j + 2];
				//}

				m_mesh->m_shapes.push_back(glm::compAdd(radiance) <= 0.0f ? 
					std::make_shared<Triangle>(tri) : 
					std::make_shared<LightTriangle>(radiance, std::make_shared<Triangle>(tri))); // Insert Lighttriangle
			}
			m_mesh->m_shapes.shrink_to_fit();
			//For editor preview render
			InitOpenGLBuffers();
		}
	}

	SceneObject::SceneObject(Mesh* mesh, std::string name, glm::vec3 pos, glm::vec3 rot, 
		glm::vec3 scale, SHAPE_T t, std::vector<unsigned int> tex_inds, glm::vec3 radiance)
	{
		SceneObject(std::make_shared<Mesh>(*mesh), name, pos, rot, scale, t, tex_inds, radiance);
	}

	SceneObject* SceneObject::CreateSphere(std::string name, std::shared_ptr<Sphere> s, glm::vec3 pos, glm::vec3 rot,
		glm::vec3 scale)
	{
		auto mesh = std::make_shared<Mesh>(*AssetImporter::LoadMeshFromOBJ("../../assets/models/sphere.obj"));
		if (mesh->GetVertexCount() != 0)
		{
			CH_WARN("Using sphere.obj file in preview-render");
		}
		SceneObject* obj = new SceneObject(mesh, name, pos, rot, scale, SHAPE_T::sphere);
		obj->m_material = s->m_material;
		s->m_visible = obj->IsVisible();
		s->SetTransform(obj->m_tranform_matrix, obj->m_inverse_tranform_matrix);

		obj->m_mesh->m_shapes.push_back(s);
		obj->m_mesh->m_shapes.shrink_to_fit();
		//For editor preview render
		obj->InitOpenGLBuffers();
		return obj;
	}

	SceneObject* SceneObject::CreateInstance(std::string name, std::shared_ptr<SceneObject> base, bool reset_transforms)
	{
		auto mesh = std::make_shared<Mesh>(base->m_mesh->m_vertex_positions, 
			base->m_mesh->m_vertex_normals, base->m_mesh->m_vertex_texcoords,
			base->m_mesh->m_vertex_colors, base->m_mesh->m_indices);

		for (int i = 0; i< base->m_mesh->m_shapes.size(); i++)//deep copy mesh
		{
			mesh->m_shapes.push_back(std::make_shared<Instance>(base->m_mesh->m_shapes[i].get(), reset_transforms));
		}

		SceneObject* instance = new SceneObject(mesh, name);
		instance->m_mesh->m_shapes.resize(instance->m_mesh->m_shapes.size() / 2);
		instance->m_mesh->m_shapes.shrink_to_fit();

		for (int i = 0; i < base->m_mesh->m_shapes.size(); i++)//deep copy mesh
		{
			mesh->m_shapes[i]->SetTransform(instance->m_tranform_matrix, instance->m_inverse_tranform_matrix);
			mesh->m_shapes[i]->m_material = instance->m_material;
		}
		if (!reset_transforms)
		{
			instance->SetTransforms(base->GetModelMatrix());
		}

		return instance;
	}

	void SceneObject::SmoothNormals()
	{
		m_mesh->SmoothNormals();

		for (auto shape : m_mesh->m_shapes)
		{
			if (shape->m_shape_type == SHAPE_T::triangle)
			{
				((Triangle*)shape.get())->m_shading_mode = SHADING_MODE::smooth;
			}
		}

		CH_TRACE("Smoothed Normals");
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
		glm::mat4 rotation = glm::eulerAngleYXZ(glm::radians(m_rotation.y),
			glm::radians(m_rotation.x), glm::radians(m_rotation.z));
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_scale);

		//scale[3][3] = 1.0f;

		*m_tranform_matrix = translation * rotation * scale;
		*m_inverse_tranform_matrix = glm::inverse(*m_tranform_matrix);
	}
	void SceneObject::InitOpenGLBuffers()
	{
		//Vertex positions buffer
		std::vector<glm::vec3> positions(m_mesh->m_vertex_positions.size()), normals(m_mesh->m_vertex_normals.size());
		std::vector<glm::vec2> uvs(m_mesh->m_vertex_texcoords.size());

		for (int i = 0; i < m_mesh->m_vertex_positions.size(); i++)
			positions[i] = *m_mesh->m_vertex_positions[i];

		for (int i = 0; i<m_mesh->m_vertex_normals.size(); i++)
			if(m_mesh->m_vertex_normals[i].get())
				normals[i] = *m_mesh->m_vertex_normals[i];

		for (int i = 0; i<m_mesh->m_vertex_texcoords.size(); i++)
			uvs[i] = *m_mesh->m_vertex_texcoords[i];

		std::shared_ptr<CHR::OpenGLVertexBuffer> position_buffer = std::make_shared<CHR::OpenGLVertexBuffer>((void*)positions.data(),
			m_mesh->m_vertex_positions.size() * sizeof(GLfloat) * 3);

		CHR::VertexAttribute layout_attribute("in_Position", CHR::Shader::POS_LAY, CHR::ShaderDataType::Float3, GL_FALSE);
		CHR::VertexBufferLayout vertex_buffer_layout;
		vertex_buffer_layout.PushAttribute(layout_attribute);
		position_buffer->SetBufferLayout(vertex_buffer_layout);

		m_vertex_buffers.push_back(position_buffer);

		//Vertex normals buffer
		std::shared_ptr<CHR::OpenGLVertexBuffer> normal_buffer = std::make_shared<CHR::OpenGLVertexBuffer>((void*)normals.data(),
			m_mesh->m_vertex_normals.size() * sizeof(GLfloat) * 3);

		CHR::VertexAttribute layout_attribute2("in_Normal", CHR::Shader::NORM_LAY, CHR::ShaderDataType::Float3, GL_FALSE);
		CHR::VertexBufferLayout vertex_buffer_layout2;
		vertex_buffer_layout2.PushAttribute(layout_attribute2);
		normal_buffer->SetBufferLayout(vertex_buffer_layout2);

		m_vertex_buffers.push_back(normal_buffer);

		//Vertex texture coords buffer
		std::shared_ptr<CHR::OpenGLVertexBuffer> tex_coord_buffer = std::make_shared<CHR::OpenGLVertexBuffer>((void*)uvs.data(),
			m_mesh->m_vertex_texcoords.size() * sizeof(GLfloat) * 2);

		CHR::VertexAttribute layout_attribute3("in_TexCoord", CHR::Shader::TEXC_LAY, CHR::ShaderDataType::Float2, GL_FALSE);
		CHR::VertexBufferLayout vertex_buffer_layout3;
		vertex_buffer_layout3.PushAttribute(layout_attribute3);
		tex_coord_buffer->SetBufferLayout(vertex_buffer_layout3);

		m_vertex_buffers.push_back(tex_coord_buffer);

		//index buffer
		std::shared_ptr<CHR::OpenGLIndexBuffer> index_buffer = std::make_shared<CHR::OpenGLIndexBuffer>(m_mesh->m_indices.data(), m_mesh->m_indices.size());
		m_index_buffer = index_buffer;

		//vertex array object
		m_vao.AddVertexBuffer(position_buffer);
		m_vao.AddVertexBuffer(normal_buffer);
		m_vao.AddVertexBuffer(tex_coord_buffer);
		m_vao.SetIndexBuffer(index_buffer);
	}
}