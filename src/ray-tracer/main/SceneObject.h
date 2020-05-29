#pragma once

#include <ray-tracer/main/ImageTextureMap.h>
#include <ray-tracer/main/Material.h>
#include <ray-tracer/main/Shape.h>
#include <ray-tracer/main/Ray.h>
#include <ray-tracer/main/Texture.h>
#include <ray-tracer/main/TextureMap.h>
#include <ray-tracer/editor/Shader.h>
#include <ray-tracer/editor/Buffer.h>
#include <ray-tracer/openGL/OpenGLBuffer.h>
#include <ray-tracer/openGL/OpenGLVertexArrayObject.h>

#include <stdio.h>
#include <string>
#include <thirdparty/glm/glm/glm.hpp>
#include <thirdparty/glm/glm/gtc/matrix_transform.hpp>
#include <thirdparty/glm/glm/gtx/matrix_decompose.hpp>
#include <thirdparty/glm/glm/gtc/quaternion.hpp>
#include <thirdparty/glm/glm/gtx/euler_angles.hpp>


namespace CHR
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
		Mesh(std::vector<std::shared_ptr<glm::vec3>> vertices, std::vector<std::shared_ptr<glm::vec3>> normals,
			std::vector<std::shared_ptr<glm::vec2>> texcoords, std::vector<std::shared_ptr<glm::vec3>> colors, unsigned int face_count, bool cntr_piv);
		Mesh(std::vector<std::shared_ptr<glm::vec3>> vertices, std::vector<std::shared_ptr<glm::vec3>> normals,
			std::vector<std::shared_ptr<glm::vec2>> texcoords, std::vector<std::shared_ptr<glm::vec3>> colors,
			std::vector<unsigned int> m_indices, bool cntr_piv = false);


		inline unsigned int GetFaceCount() const { return m_face_count; }
		inline unsigned int GetVertexCount() const { return m_vertex_count; }
		inline glm::vec3 GetMaxBound() const { return m_bound_max; }
		inline glm::vec3 GetMinBound() const { return m_bound_min; }

		inline void SetFaceCount(unsigned int f_c) { m_face_count = f_c; }
		inline void SetVertexCount(unsigned int v_c) { m_vertex_count = v_c; }
		inline void SetMaxBound(glm::vec3 max_b) { m_bound_max = max_b; }
		inline void SetMinBound(glm::vec3 min_b) { m_bound_min = min_b; }

		void SmoothNormals();

		std::vector<std::shared_ptr<glm::vec3>> m_vertex_positions;
		std::vector<std::shared_ptr<glm::vec3>> m_vertex_normals;
		std::vector<std::shared_ptr<glm::vec3>> m_vertex_colors;
		std::vector<std::shared_ptr<glm::vec2>> m_vertex_texcoords;

		std::vector<unsigned int> m_indices;
		std::vector<std::shared_ptr<Shape>> m_shapes;
		SHADING_MODE m_shading_mode = SHADING_MODE::flat;

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

	class SceneObject : public ImGuiDrawable
	{
	public:
		SceneObject(std::shared_ptr<Mesh> mesh, std::string name,
			glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), 
			SHAPE_T t = SHAPE_T::triangle, std::vector<unsigned int> tex_inds = std::vector<unsigned int>());
		SceneObject(Mesh* mesh, std::string name,
			glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f),
			SHAPE_T t = SHAPE_T::triangle, std::vector<unsigned int> tex_inds = std::vector<unsigned int>());
		//~SceneObject();

		static SceneObject* CreateSphere(std::string name, Sphere s, glm::vec3 pos, glm::vec3 rot,
			glm::vec3 scale);
		static SceneObject* CreateInstance(std::string name, std::shared_ptr<SceneObject> base, bool reset_transforms = false);

		void SmoothNormals();


		inline void HideInEditor(bool hide) { m_visible_in_editor = hide; }
		inline void SetVisible(bool visible) 
		{ 
			m_visible = visible; 
			for (auto shape : m_mesh->m_shapes)
				shape->m_visible = visible;
		}
		inline void SetPickable(bool pickable) { m_pickable = pickable; }

		inline bool IsVisibleInEditor() const { return m_visible_in_editor; }
		inline bool IsVisible() const { return m_visible; }
		inline bool IsPickable() const { return m_pickable; }


		inline void SetPosition(const glm::vec3 pos) { m_position = pos; RecalculateModelMatrix(); }
		inline void SetRotation(const glm::vec3 rot) { m_rotation = rot; RecalculateModelMatrix(); }
		inline void SetScale(const glm::vec3 sca) { m_scale = sca; RecalculateModelMatrix(); }

		inline glm::mat4 GetModelMatrix() const { return *m_tranform_matrix; }
		inline glm::vec3 GetPosition() const { return m_position; }
		inline glm::vec3 GetRotation() const { return m_rotation; }
		inline glm::vec3 GetScale() const { return m_scale; }

		inline glm::vec3 GetMotionBlur() { return m_motion_blur; }

		inline void Translate(const glm::vec3 vec) { m_position += vec; RecalculateModelMatrix(); }
		/*inline void RotateAngleAxis(const float angle, const glm::vec3 axis) {
			m_rotation = glm::angleAxis(angle, axis) * m_rotation;
			RecalculateModelMatrix();
		}
		inline void RotateEuler(const glm::vec3 euler_angles) {
			m_rotation = glm::quat(euler_angles) * m_rotation;
			RecalculateModelMatrix();
		}*/
		inline void SetTransforms(glm::mat4 mat) 
		{
			glm::quat rot;
			glm::vec3 dummy;
			glm::vec4 dummy2;
			glm::decompose(mat, m_scale, rot, m_position, dummy, dummy2);
			*m_tranform_matrix = mat; 
			*m_inverse_tranform_matrix = glm::inverse(mat);
			glm::vec3 rad_angles = glm::eulerAngles(rot);
			m_rotation = { glm::degrees(rad_angles) };
		}
		inline void Scale(const glm::vec3 scale) { m_scale *= scale; RecalculateModelMatrix(); }
		inline void ResetTransforms() { *m_tranform_matrix = glm::mat4(1.0f); }


		inline void SetTexture(Texture tex) { m_texture = tex; }

		inline void SetTextureMap(std::shared_ptr<TextureMap> tex_map)
		{
			int ind = 0;
			ind = tex_map->GetDecalMode() == DECAL_M::re_no || 
				tex_map->GetDecalMode() == DECAL_M::bump ? 1 : ind;

			if(tex_map.get()->GetType() == SOURCE_T::image && ind == 0)
				m_texture = *((static_cast<ImageTextureMap*>(tex_map.get())->m_texture).get());
			/*else if(tex_map.get()->GetType() == SOURCE_T::image)
				m_texture = Chroma::Texture("../../assets/textures/white.png");*/

			for (auto shape : m_mesh->m_shapes)
				shape->m_tex_maps[ind] = tex_map;
		}

		inline void SetMaterial(std::shared_ptr<Material> mat) {
			m_material = mat; 
			for (auto shape : m_mesh->m_shapes)
				shape->m_material = mat;
		}

		inline std::shared_ptr<Material> GetMaterial() const { return m_material; }

		inline SHAPE_T GetShapeType() { return m_shape_t; }

		//inline std::string GetName() const { return m_name; }
		//inline void SetName(std::string n) { m_name = n; }
		inline void SetMotionBlur(glm::vec3 mb) 
		{ 
			m_motion_blur = mb; 
			for (auto shape : m_mesh->m_shapes)
				shape->m_motion_blur = mb;
		}

		void DrawGUI()
		{
			ImGui::PushStyleColor(ImGuiCol_Header, CHR_COLOR::O_FRAME);
			ImGui::CollapsingHeader(std::string("Scene Object: " + m_name).c_str(), ImGuiTreeNodeFlags_Leaf);
			ImGui::PopStyleColor();

			ImGui::Checkbox("RT visibility", &m_visible);
			ImGui::Checkbox("Editor visibility", &m_visible_in_editor);
			ImGui::Separator();
			
			static bool flag = true;
			if (flag)
				ImGui::SetNextTreeNodeOpen(true);
			if (ImGui::CollapsingHeader("Transform"))
			{

				if (ImGui::Button("P##1"))SetPosition(glm::vec3());
				ImGui::SameLine();
				glm::vec3 tmp_pos = GetPosition();
				ImGui::DragFloat3("##4", &(tmp_pos.x), 0.05f, 0, 0, "%.3f");
				SetPosition(tmp_pos);

				if (ImGui::Button("R##2"))SetRotation(glm::vec3());
				ImGui::SameLine();
				glm::vec3 tmp_rot = GetRotation();
				ImGui::DragFloat3("##5", &(tmp_rot.x), 0.25f, 0, 0, "%.3f");
				SetRotation(tmp_rot);

				if (ImGui::Button("S##3")) SetScale(glm::vec3(1, 1, 1));
				ImGui::SameLine();
				glm::vec3 tmp_sca = GetScale();
				ImGui::DragFloat3("##6", &(tmp_sca.x), 0.05f, 0, 0, "%.3f");
				SetScale(tmp_sca);

				ImGui::Separator();

				glm::vec3 tmp_mb = GetMotionBlur();
				ImGui::DragFloat3("Motion Blur", &(tmp_mb.x), 0.05f, 0, 0, "%.3f");
				SetMotionBlur(tmp_mb);
				ImGui::Separator();
			}


			if (flag)
				ImGui::SetNextTreeNodeOpen(true);
			if (ImGui::CollapsingHeader("Material"))
			{
				Material* mat = GetMaterial().get();
				ImGui::DragFloat3("Ambient Ref.", &(mat->m_ambient.x), 0.002f, 0.0f, 1.0f, "%.3f");
				ImGui::DragFloat3("Diffuse Ref.", &(mat->m_diffuse.x), 0.002f, 0.0f, 1.0f, "%.3f");
				ImGui::DragFloat3("Specular Ref.", &(mat->m_specular.x), 0.002f, 0.0f, 1.0f, "%.3f");
				ImGui::DragFloat("Phong Exp.", &(mat->m_shininess), 0.002f, 0.0f, 1.0f, "%.3f");
				ImGui::DragFloat("Roughness", &(mat->m_roughness), 0.002f, 0.0f, 1.0f, "%.3f");
				ImGui::Separator();
				static std::string mat_names[] = { "Diffuse", "Mirror", "Dielectric", "Conductor" };
				static int selected_mat_type;
				if (ImGui::BeginCombo("Type", mat_names[static_cast<int>(mat->type)].c_str(), ImGuiComboFlags_None))
				{
					for (int i = 0; i < 4; i++)
					{
						if (ImGui::Selectable(mat_names[i].c_str(), mat->type == static_cast<MAT_TYPE>(selected_mat_type)))
						{
							selected_mat_type = i;

							std::shared_ptr<Material> mat2;

							switch (static_cast<MAT_TYPE>(selected_mat_type))
							{
							case MAT_TYPE::conductor:
								mat2 = std::make_shared<Conductor>(*mat);
								static_cast<Conductor*>(mat2.get())->m_absorption_ind = 1.0f;
								static_cast<Conductor*>(mat2.get())->m_mirror_reflec = glm::vec3(1, 1, 1);
								static_cast<Conductor*>(mat2.get())->m_refraction_ind = 1.2f;
								break;
							case MAT_TYPE::dielectric:
								mat2 = std::make_shared<Dielectric>(*mat);
								static_cast<Dielectric*>(mat2.get())->m_refraction_ind = 1.2f;
								static_cast<Dielectric*>(mat2.get())->m_absorption_coeff = glm::vec3(0, 0, 0);
								break;
							case MAT_TYPE::mirror:
								mat2 = std::make_shared <Mirror>(*mat);
								static_cast<Mirror*>(mat2.get())->m_mirror_reflec = glm::vec3(1, 1, 1);
								break;
							default:
								mat2 = std::make_shared<Material>(*mat);
								break;
							}
							SetMaterial(mat2);
							//delete mat;

						}
						if (mat->type == static_cast<MAT_TYPE>(selected_mat_type))
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}


				if (mat->type == MAT_TYPE::conductor)
				{
					ImGui::DragFloat3("Mirror Ref.", &(((Conductor*)mat)->m_mirror_reflec.x), 0.002f, 0.0f, 1.0f, "%.3f");
					ImGui::DragFloat("Absorp Ind.", &(((Conductor*)mat)->m_absorption_ind), 0.002f, 0.0f, 0.0f, "%.3f");
					ImGui::DragFloat("Refraction Ind.", &(((Conductor*)mat)->m_refraction_ind), 0.002f, 0.0f, 0.0f, "%.3f");
				}
				else if (mat->type == MAT_TYPE::dielectric)
				{
					ImGui::DragFloat3("Absorp Coef.", &(((Dielectric*)mat)->m_absorption_coeff.x), 0.002f, 0.0f, 1.0f, "%.3f");
					ImGui::DragFloat("Refraction Ind.", &(((Dielectric*)mat)->m_refraction_ind), 0.002f, 0.0f, 0.0f, "%.3f");
				}
				else if (mat->type == MAT_TYPE::mirror)
				{
					ImGui::DragFloat3("Mirror Ref.", &(((Mirror*)mat)->m_mirror_reflec.x), 0.002f, 0.0f, 1.0f, "%.3f");
				}
			}

			ImGui::Separator();
			auto shape = m_mesh->m_shapes[0];
			TextureMap* tex_map;
			if (tex_map = shape->m_tex_maps[0].get())
			{
				if (ImGui::CollapsingHeader("Texture Map"))
					tex_map->DrawGUI();
			}
			if (tex_map = shape->m_tex_maps[1].get())
			{
				if (ImGui::CollapsingHeader("Normal Map"))
					tex_map->DrawGUI();
			}

			flag = false;
		}

		void Draw(DrawMode mode);

		//float m_radius;
		std::shared_ptr<Mesh> m_mesh;

	private:
		void RecalculateModelMatrix();
		void InitOpenGLBuffers();

		bool m_visible_in_editor = true;
		bool m_visible = true;
		bool m_pickable = true;

		glm::vec3 m_position;
		glm::vec3 m_rotation;
		glm::vec3 m_scale;

		glm::vec3 m_motion_blur = { 0,0,0 };

		glm::mat4* m_tranform_matrix = new glm::mat4(1.0);
		glm::mat4* m_inverse_tranform_matrix = new glm::mat4(1.0);

		CHR::Texture m_texture;
		std::shared_ptr<Material> m_material;

		SHAPE_T m_shape_t;

		CHR::OpenGLVertexArrayObject m_vao;
		std::vector<std::shared_ptr<CHR::VertexBuffer>> m_vertex_buffers;
		std::shared_ptr<CHR::IndexBuffer> m_index_buffer;
	};
}

