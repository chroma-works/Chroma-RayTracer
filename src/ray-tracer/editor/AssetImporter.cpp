#include "AssetImporter.h"

#include <iostream>
#include <sstream>
#include <string> 

#include <ray-tracer/editor/Logger.h>
#include <ray-tracer/main/Shape.h>
#include <ray-tracer/main/Material.h>

#include <thirdparty/hapPLY/happly.h>
#include <thirdparty/OBJ_loader/OBJ_Loader.h>
#include <thirdparty/glm/glm/gtx/quaternion.hpp>


namespace Chroma
{

	const std::string ABS_COEF = "AbsorptionCoefficient";
	const std::string ABS_IND = "AbsorptionIndex";
	const std::string AM_LIG = "AmbientLight";//INIT as point light with very low constans
	const std::string AM_REF = "AmbientReflectance";
	const std::string BCK_COLOR = "BackgroundColor";
	const std::string CAMS = "Cameras";
	const std::string CNTR = "Center";
	const std::string DIF_REF = "DiffuseReflectance";
	const std::string FACES = "Faces";
	const std::string GAZE = "Gaze";
	const std::string I_TEST_EPS = "IntersectionTestEpsilon";
	const std::string IM_NAME = "ImageName";
	const std::string IND = "Indices";
	const std::string INTEN = "Intensity";
	const std::string LIGS = "Lights";
	const std::string MAT = "Material";
	const std::string MATS = "Materials";
	const std::string MAX_RECUR = "MaxRecursionDepth";
	const std::string MESH = "Mesh";
	const std::string MIRROR_REF = "MirrorReflectance";
	const std::string N_DIST = "NearDistance";
	const std::string N_PLANE = "NearPlane";
	const std::string NUM_SAMP = "NumSamples";
	const std::string OBJ = "Objects";
	const std::string UP = "Up";
	const std::string P_LIG = "PointLight";
	const std::string PHONG_EX = "PhongExponent";
	const std::string POS = "Position";
	const std::string RAD = "Radius";
	const std::string REF_IND = "RefractionIndex";
	const std::string RES = "ImageResolution";
	const std::string ROT = "Rotation";
	const std::string S_RAY_EPS = "ShadowRayEpsilon";
	const std::string SPEC_REF = "SpecularReflectance";
	const std::string SPHR = "Sphere";
	const std::string TRIANGLE = "Triangle";
	const std::string TRANSFORMS = "Transformations";
	const std::string TRA = "Translation";
	const std::string SCA = "Scaling";
	const std::string VRTX_DATA = "VertexData";

    Mesh* AssetImporter::LoadMeshFromOBJ(const std::string& file_name, 
		glm::vec3 t, glm::vec3 r, glm::vec3 s)
    {
        Mesh* mesh = new Mesh();

        objl::Loader Loader;

        // Load .obj File
        bool loadout = Loader.LoadFile(file_name);
        if (loadout)
        {
            objl::Mesh curMesh = Loader.LoadedMeshes[0];
            for (int j = 0; j < curMesh.Vertices.size(); j++)
            {
                mesh->m_vertex_positions.push_back(glm::vec3(glm::eulerAngleYXZ(r.y, r.x, r.z) * glm::vec4(glm::vec3(curMesh.Vertices[j].Position.X,
                    curMesh.Vertices[j].Position.Y, 
                    curMesh.Vertices[j].Position.Z )*s + t, 1.0f)));
                mesh->m_vertex_normals.push_back(glm::vec3(glm::eulerAngleYXZ(r.y, r.x, r.z) * glm::vec4(curMesh.Vertices[j].Normal.X,
                    curMesh.Vertices[j].Normal.Y,
                    curMesh.Vertices[j].Normal.Z,1)));
                mesh->m_vertex_texcoords.push_back(glm::vec2(curMesh.Vertices[j].TextureCoordinate.X, 
                    curMesh.Vertices[j].TextureCoordinate.Y ));
            }

            mesh->m_indices = curMesh.Indices;
			mesh->SetFaceCount(mesh->m_indices.size() / 3);

        }

        return mesh;
    }

    Texture* AssetImporter::LoadTexture(const std::string& file_name)
    {
        return new Texture(file_name);
    }

	size_t Split(const std::string& txt, std::vector<std::string>& strs, char ch)
	{
		size_t pos = txt.find(ch);
		size_t initialPos = 0;
		strs.clear();

		// Decompose statement
		while (pos != std::string::npos) {
			strs.push_back(txt.substr(initialPos, pos - initialPos));
			initialPos = pos + 1;

			pos = txt.find(ch, initialPos);
		}

		// Add the last one
		strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

		return strs.size();
	}

	glm::mat4 CalculateTransforms(
		std::vector<glm::mat4> translations,
		std::vector<glm::mat4> rotations, 
		std::vector<glm::mat4> scalings,
		tinyxml2::XMLNode* node)
	{
		std::string data = node->FirstChild()->Value();
		std::vector<std::string> v;

		Split(data, v, ' ');
		glm::mat4 trnsfm = glm::mat4(1.0f);

		for (int i = 0; i < v.size(); i++)
		{
			if (v[i].at(0) == 't')
			{
				trnsfm *= translations[ std::stoi(v[i].substr(1)) -1];
			}
			else if (v[i].at(0) == 'r')
			{
				trnsfm *= rotations[ std::stoi(v[i].substr(1)) -1];
			}
			else if (v[i].at(0) == 's')
			{
				trnsfm *= scalings[ std::stoi(v[i].substr(1)) -1];
			}
		}
		return trnsfm;
	}

	Mesh* ParsePly(std::string ply_path)
	{
		std::vector<glm::vec3> mesh_verts;
		std::vector<glm::vec2> mesh_uvs;
		std::vector<glm::vec3> mesh_normals;
		std::vector<unsigned int> mesh_indices;

		happly::PLYData ply_in(ply_path);
		std::vector<std::array<double, 3>> v_pos = ply_in.getVertexPositions();
		std::vector<std::vector<size_t>> f_ind = ply_in.getFaceIndices<size_t>();

		for (int i = 0; i < v_pos.size(); i++)
		{
			mesh_verts.push_back({ v_pos[i][0], v_pos[i][1], v_pos[i][2] });
			/*mesh_verts.push_back({ v_pos[i+1][0], v_pos[i+1][1], v_pos[i+1][2] });
			mesh_verts.push_back({ v_pos[i+2][0], v_pos[i+2][1], v_pos[i+2][2] });*/
		}
		for (auto faces : f_ind)
		{
			glm::vec3 a = (mesh_verts[faces[1]] - mesh_verts[faces[0]]);
			glm::vec3 b = (mesh_verts[faces[2]] - mesh_verts[faces[0]]);
			glm::vec3 normal = glm::normalize(glm::cross(-a, -b));
			if(faces.size() ==3)		//triangle faces
				for (int i = 0; i < faces.size(); i ++)
				{
					mesh_normals.push_back(normal);
					mesh_indices.push_back(faces[i]);
				}
			else if(faces.size() == 4)	//quad faces
			{
				for (int start_ind = 1; start_ind+2 <= faces.size(); start_ind++)
				{
					mesh_normals.push_back(normal);
					mesh_indices.push_back(faces[0]);
					for (int i = start_ind; i < start_ind + 2; i++)
					{
						mesh_normals.push_back(normal);
						mesh_indices.push_back(faces[i]);
					}
				}
			}

		}
		return new Mesh(mesh_verts, mesh_normals, mesh_uvs, std::vector<glm::vec3>(), mesh_indices);
	}

	Camera* ParseCamera(tinyxml2::XMLNode* node)
	{
		Camera* cam = new Camera(1.0f * 1280,
			1.0f * 720, 0.1f, 300.0f, 60.0f); //Standart Camera(raster pipeline parameters)
		tinyxml2::XMLNode* cam_prop = node->FirstChild();

		auto cam_type = node->ToElement()->FindAttribute("type");

		while (cam_prop)//iterate over each cameras properties
		{
			//--------------Shared properties---------------
			if (std::string(cam_prop->Value()).compare(POS) == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				glm::vec3 vec({ 0,0,0 });
				sscanf(data.c_str(), "%f %f %f", &vec.x, &vec.y, &vec.z);
				cam->SetPosition(vec);
			}
			else if (std::string(cam_prop->Value()).compare(UP) == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				glm::vec3 vec({ 0,0,0 });
				sscanf(data.c_str(), "%f %f %f", &vec.x, &vec.y, &vec.z);
				cam->SetUp(vec);
			}
			else if (std::string(cam_prop->Value()).compare(N_DIST) == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				float d;
				sscanf(data.c_str(), "%f", &d);
				cam->SetNearDist(d);
			}
			else if (std::string(cam_prop->Value()).compare(RES) == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				glm::ivec2 tmp;
				sscanf(data.c_str(), "%d %d", &tmp.x, &tmp.y);
				cam->SetResolution(tmp);
			}
			else if (std::string(cam_prop->Value()).compare(IM_NAME) == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				cam->SetImageName(data);
			}
			else if (std::string(cam_prop->Value()).compare(NUM_SAMP) == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				unsigned int tmp;
				sscanf(data.c_str(), "%d", &tmp);
				cam->SetNumberOfSamples(tmp);
			}
			//----------------Typeless cam-------------------
			else if (!cam_type &&
				std::string(cam_prop->Value()).compare(N_PLANE) == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				glm::vec2 vec[2];
				sscanf(data.c_str(), "%f %f %f %f", &vec[0].x, &vec[1].x, &vec[1].y, &vec[0].y);
				cam->SetNearPlane(vec);
			}
			else if (!cam_type &&
				std::string(cam_prop->Value()).compare(GAZE) == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				glm::vec3 vec({ 0,0,0 });
				sscanf(data.c_str(), "%f %f %f", &vec.x, &vec.y, &vec.z);
				cam->SetGaze(vec);
			}
			//----------------Look at cam------------------
			else if ((std::string(cam_type->Value()).compare("lookAt") == 0) &&
				std::string(cam_prop->Value()).compare(POS) == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				glm::vec3 vec({ 0,0,0 });
				sscanf(data.c_str(), "%f %f %f", &vec.x, &vec.y, &vec.z);
				cam->SetPosition(vec);
			}
			else if ((std::string(cam_type->Value()).compare("lookAt") == 0) &&
				std::string(cam_prop->Value()).compare("GazePoint") == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				glm::vec3 vec({ 0,0,0 });
				sscanf(data.c_str(), "%f %f %f", &vec.x, &vec.y, &vec.z);
				cam->SetGaze(glm::normalize(vec - cam->GetPosition()));
			}
			else if ((std::string(cam_type->Value()).compare("lookAt") == 0) &&
				std::string(cam_prop->Value()).compare("FovY") == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				float fovy;
				sscanf(data.c_str(), "%f", &fovy);
				float aspect_ratio;
				glm::vec2 vec[2];
				vec[0].x = -0.5f;
				vec[1].x = -vec[0].x;
				vec[0].y = std::tan(fovy * 0.5f);
				vec[1].y = -std::tan(fovy * 0.5f);
				cam->SetNearPlane(vec);

			}
			cam_prop = cam_prop->NextSibling();
		}
		if (glm::dot(cam->GetGaze(), cam->GetUp()) != 0)
		{
			CH_INFO("Camera Up and gaze are not orthogonal. Recalculating up...");
			glm::vec3 u = glm::cross(cam->GetGaze(), cam->GetUp());
			glm::vec3 v_prime = glm::cross(-cam->GetGaze(), u);
			cam->SetUp(v_prime);
		}
		return cam;
	}

	Scene* AssetImporter::LoadSceneFromXML(Shader* shader, const std::string& file_path)
	{
		//Scene name from file name
		size_t found = file_path.find_last_of("/\\");
		size_t found2 = file_path.find_last_of(".");
		std::string file_name = file_path.substr(found+1, found2-found-1);

		Scene* scene = new Scene(file_name, shader);

		tinyxml2::XMLDocument doc;
		doc.LoadFile(file_path.c_str());
		std::vector<Material*> materials;
		std::vector<glm::vec3> vertices;

		std::vector<glm::mat4> translations;
		std::vector<glm::mat4> rotations;
		std::vector<glm::mat4> scalings;

		tinyxml2::XMLNode* node = doc.RootElement()->FirstChild();
		while (node)
		{
			if (std::string(node->Value()).compare(MAX_RECUR) == 0)
			{
				std::string data = node->FirstChild()->Value();
				sscanf(data.c_str(), "%d", &scene->m_recur_dept);
			}
			else if (std::string(node->Value()).compare(BCK_COLOR) == 0)
			{
				std::string data = node->FirstChild()->Value();
				sscanf(data.c_str(), "%f %f %f",&scene->m_sky_color.x, &scene->m_sky_color.y, &scene->m_sky_color.z);
			}
			else if (std::string(node->Value()).compare(S_RAY_EPS) == 0)
			{
				std::string data = node->FirstChild()->Value();
				sscanf(data.c_str(), "%f", &scene->m_shadow_eps);
			}
			else if (std::string(node->Value()).compare(I_TEST_EPS) == 0)
			{
				std::string data = node->FirstChild()->Value();
				sscanf(data.c_str(), "%f", &scene->m_intersect_eps);
			}
			else if (std::string(node->Value()).compare(CAMS) == 0)
			{
				tinyxml2::XMLNode* child_node = node->FirstChild();
				while (child_node)//iterate over cameras
				{
					std::string cam_name = "camera_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
					Camera* cam = ParseCamera( child_node);

					scene->AddCamera(cam_name, cam);
					child_node = child_node->NextSibling();
				}
			}
			else if (std::string(node->Value()).compare(LIGS) == 0)
				{
				tinyxml2::XMLNode* child_node = node->FirstChild();
				while (child_node)//iterate over lights
				{
					if (std::string(child_node->Value()).compare(AM_LIG) == 0)
					{
						std::string data = child_node->FirstChild()->Value();

						sscanf(data.c_str(), "%f %f %f", &scene->m_ambient_l.x, &scene->m_ambient_l.y, &scene->m_ambient_l.z);

					}
					else if (std::string(child_node->Value()).compare(P_LIG) == 0)
					{
						PointLight p_l = PointLight({ 0,0,0 }, { 0,0,0 }, { 0,0,0 }, { 0,0,0 });
						std::string lig_name;
						lig_name = "point_light_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
						tinyxml2::XMLNode* lig_prop = child_node->FirstChild();

						while (lig_prop)//iterate over each light properties
						{
							if (std::string(lig_prop->Value()).compare(POS) == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f %f %f", &p_l.position.x, &p_l.position.y, &p_l.position.z);
							}
							else if (std::string(lig_prop->Value()).compare(INTEN) == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								glm::vec3 tmp({ 10,10,10 });
								sscanf(data.c_str(), "%f %f %f", &tmp.x, &tmp.y, &tmp.z);

								p_l.SetIntensity(tmp);
								p_l.ambient = glm::clamp(scene->m_ambient_l / 1000.0f, 0.0f, 1.0f);//Reset ambient light to approixmate ray traced env
							}
							lig_prop = lig_prop->NextSibling();
						}
						scene->AddLight(lig_name, std::make_shared<PointLight>(p_l));

					}
					child_node = child_node->NextSibling();
				}
			}
			else if (std::string(node->Value()).compare(TRANSFORMS) == 0)
			{
				tinyxml2::XMLNode* child_node = node->FirstChild();
				while (child_node)//iterate over Transformations
				{
					glm::mat4 t = glm::mat4(1.0f);
					if (std::string(child_node->Value()).compare(TRA) == 0)
					{
						glm::vec3 temp;
						std::string data = child_node->FirstChild()->Value();
						sscanf(data.c_str(), "%f %f %f", &temp.x, &temp.y, &temp.z);
						t = glm::translate(glm::mat4(1.0f), temp);
						translations.push_back(t);
					}
					else if (std::string(child_node->Value()).compare(ROT) == 0)
					{
						glm::vec3 axis;
						float angle_d;
						std::string data = child_node->FirstChild()->Value();
						sscanf(data.c_str(), "%f %f %f %f", &angle_d, &axis.x, &axis.y, &axis.z);
						t = glm::toMat4(glm::angleAxis(glm::radians(angle_d), glm::normalize(axis)));
						rotations.push_back(t);
					}
					else if (std::string(child_node->Value()).compare(SCA) == 0)
					{
						glm::vec3 temp;
						std::string data = child_node->FirstChild()->Value();
						sscanf(data.c_str(), "%f %f %f", &temp.x, &temp.y, &temp.z);
						t = glm::scale(glm::mat4(1.0f), temp);
						scalings.push_back(t);
					}
					child_node = child_node->NextSibling();
				}
			}
			else if (std::string(node->Value()).compare(MATS) == 0)
			{
				tinyxml2::XMLNode* child_node = node->FirstChild();
				while (child_node)//iterate over Materials
				{
					Material* mat;
					MAT_TYPE type = MAT_TYPE::none;
					bool flag = true;
					tinyxml2::XMLNode* material_prop = child_node->FirstChild();
					if (child_node->ToElement()->FindAttribute("type"))
					{
						std::string type_string = std::string(child_node->ToElement()->FindAttribute("type")->Value());//get type
						type = type_string.empty() ? MAT_TYPE::none :
							type_string.compare("conductor") == 0 ? MAT_TYPE::conductor :
							type_string.compare("mirror") == 0 ? MAT_TYPE::mirror : MAT_TYPE::dielectric;
					}
					while (material_prop)
					{
						switch (type)
						{
							case MAT_TYPE::mirror:
							{
								if(flag)
									mat = new Mirror(); flag = false;
								if (std::string(material_prop->Value()).compare(MIRROR_REF) == 0)
								{
									std::string data = material_prop->FirstChild()->Value();
									sscanf(data.c_str(), "%f %f %f", &(static_cast<Mirror*>(mat))->m_mirror_reflec.x,
										&(static_cast<Mirror*>(mat))->m_mirror_reflec.y,
										&(static_cast<Mirror*>(mat))->m_mirror_reflec.z);
								}
							}
								break;
							case MAT_TYPE::conductor:
							{
								if (flag)
									mat = new Conductor(); flag = false;
								if (std::string(material_prop->Value()).compare(MIRROR_REF) == 0)
								{
									std::string data = material_prop->FirstChild()->Value();
									sscanf(data.c_str(), "%f %f %f", &(static_cast<Conductor*>(mat))->m_mirror_reflec.x,
										&(static_cast<Conductor*>(mat))->m_mirror_reflec.y,
										&(static_cast<Conductor*>(mat))->m_mirror_reflec.z);
								}
								else if (std::string(material_prop->Value()).compare(REF_IND) == 0)
								{
									std::string data = material_prop->FirstChild()->Value();
									sscanf(data.c_str(), "%f", &(static_cast<Conductor*>(mat))->m_refraction_ind);
								}
								else if (std::string(material_prop->Value()).compare(ABS_IND) == 0)
								{
									std::string data = material_prop->FirstChild()->Value();
									sscanf(data.c_str(), "%f", &(static_cast<Conductor*>(mat))->m_absorption_ind);
								}
							}
								break;
							case MAT_TYPE::dielectric:
							{
								if (flag)
									mat = new Dielectric(); flag = false;
								if (std::string(material_prop->Value()).compare(REF_IND) == 0)
								{
									std::string data = material_prop->FirstChild()->Value();
									sscanf(data.c_str(), "%f", &(static_cast<Dielectric*>(mat))->m_refraction_ind);
								}
								else if (std::string(material_prop->Value()).compare(ABS_COEF) == 0)
								{
									std::string data = material_prop->FirstChild()->Value();
									sscanf(data.c_str(), "%f %f %f", &(static_cast<Dielectric*>(mat))->m_absorption_coeff.x,
										&(static_cast<Dielectric*>(mat))->m_absorption_coeff.y,
										&(static_cast<Dielectric*>(mat))->m_absorption_coeff.z);
								}
							}
								break;
							default:
							{
								if (flag)
									mat = new Material(); flag = false;
							}
								break;
						}
						if (std::string(material_prop->Value()).compare(AM_REF) == 0)
						{
							std::string data = material_prop->FirstChild()->Value();
							sscanf(data.c_str(), "%f %f %f", &mat->m_ambient.x, &mat->m_ambient.y, &mat->m_ambient.z);
						}
						else if (std::string(material_prop->Value()).compare(DIF_REF) == 0)
						{
							std::string data = material_prop->FirstChild()->Value();
							sscanf(data.c_str(), "%f %f %f", &mat->m_diffuse.x, &mat->m_diffuse.y, &mat->m_diffuse.z);
						}
						else if (std::string(material_prop->Value()).compare(SPEC_REF) == 0)
						{
							std::string data = material_prop->FirstChild()->Value();
							sscanf(data.c_str(), "%f %f %f", &mat->m_specular.x, &mat->m_specular.y, &mat->m_specular.z);
						}
						else if (std::string(material_prop->Value()).compare(PHONG_EX) == 0)
						{
							std::string data = material_prop->FirstChild()->Value();
							sscanf(data.c_str(), "%f", &mat->m_shininess);
						}

						material_prop = material_prop->NextSibling();
					}

					child_node = child_node->NextSibling();
					materials.push_back(mat);
				}
			}
			else if (std::string(node->Value()).compare(VRTX_DATA) == 0)
			{
				tinyxml2::XMLNode* child_node = node->FirstChild();

				std::string data = child_node->Value();
				std::string line;
				std::istringstream stream(data);

				CH_TRACE("Parsing vertices");

				while (std::getline(stream, line)) {//read vertices line by line
					glm::vec3 vertex;
					std::istringstream iss(line);
					iss >> vertex.x >> vertex.y >> vertex.z;
					//sscanf(line.c_str(), "%f %f %f", &vertex.x, &vertex.y, &vertex.z);

					if (iss)
						vertices.push_back(vertex);
				}
			}
			else if (std::string(node->Value()).compare(OBJ) == 0)
			{
				CH_TRACE("Parsing objects");
				tinyxml2::XMLNode* child_node = node->FirstChild();
				while (child_node)
				{
					if(std::string(child_node->Value()).compare(MESH) == 0)
					{ 
						tinyxml2::XMLNode* object_prop = child_node->FirstChild();
						std::string name = "scene_object_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
						Mesh* mesh = nullptr;
						int mat_ind = 0;
						bool ply_parsed = false;
						glm::mat4 transform = glm::mat4(1.0f);

						while (object_prop)
						{
							if (std::string(object_prop->Value()).compare(MAT) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%d", &mat_ind);
								mat_ind = mat_ind - 1;
							}
							else if (std::string(object_prop->Value()).compare(FACES) == 0)
							{
								auto ply_file_path = object_prop->ToElement()->FindAttribute("plyFile");
								if (ply_file_path)
								{
									mesh = ParsePly(file_path.substr(0, found + 1) + std::string(ply_file_path->Value()));
									//mesh = new Mesh(mesh_verts, mesh_normals, mesh_uvs, std::vector<glm::vec3>(), mesh_indices);
								}
								else
								{
									std::string data = object_prop->FirstChild()->Value();
									std::vector<glm::vec3> mesh_verts;
									std::vector<glm::vec2> mesh_uvs;
									std::vector<glm::vec3> mesh_normals;
									std::vector<unsigned int> mesh_indices;

									std::string line;
									std::istringstream stream(data);
									mesh_verts = vertices;
									mesh_normals.reserve(vertices.size());
									mesh_normals.resize(vertices.size());

									while (std::getline(stream, line)) //read faces line by line
									{
										unsigned int ind[3];
										std::istringstream iss(line);
										iss >> ind[0] >> ind[1] >> ind[2];

										if (iss) {

											glm::vec3 a = (vertices[ind[0] - 1] - vertices[ind[1] - 1]);
											glm::vec3 b = (vertices[ind[0] - 1] - vertices[ind[2] - 1]);

											glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);//glm::normalize(glm::cross(a, b));//calculate face normal

											normal = (glm::cross(-a, -b));

											for (int j = 0; j < 3; j++)
											{
												mesh_indices.push_back(ind[j] - 1);
												if (std::isnan(mesh_normals[ind[j] - 1].x))
												{
													mesh_normals[ind[j] - 1] = (normal);
												}
												else
												{
													mesh_normals[ind[j] - 1] += normal;
												}

											}
										}
										/*for (int i = 0; i < mesh_normals.size(); i++)
										{
											mesh_normals[i] = glm::normalize(mesh_normals[i]); /// (float)num_shared_faces[i];//average vert. normals for shared faces
										}*/
									}
									mesh = new Mesh(mesh_verts, mesh_normals, mesh_uvs, std::vector<glm::vec3>(), mesh_indices);
								}

							}
							else if (std::string(object_prop->Value()).compare(TRANSFORMS) == 0)
							{
								transform = CalculateTransforms(translations, rotations,scalings, object_prop);
							}
							object_prop = object_prop->NextSibling();
						}
						SceneObject* scene_obj = new SceneObject(std::make_shared<Mesh>(*mesh), name, glm::vec3(), glm::vec3(), glm::vec3(1.0, 1.0, 1.0), SHAPE_T::triangle);
						scene_obj->SetMaterial(materials[mat_ind]);
						scene_obj->SetTransforms(transform);
						//CH_TRACE(glm::to_string(scene_obj->GetMaterial()->diffuse));
						scene->AddSceneObject(scene_obj->GetName(), std::make_shared<SceneObject>(*scene_obj));
					}
					else if (std::string(child_node->Value()).compare(TRIANGLE) == 0)
					{
						tinyxml2::XMLNode* object_prop = child_node->FirstChild();
						std::string name = "triangle_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
						Mesh* mesh = nullptr;
						int mat_ind = 0;
						glm::mat4 transform = glm::mat4(1.0f);

						while (object_prop)
						{
							if (std::string(object_prop->Value()).compare(MAT) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%d", &mat_ind);
								mat_ind = mat_ind - 1;
							}
							else if (std::string(object_prop->Value()).compare(IND) == 0)
							{
								int indices[3];
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%d %d %d", &indices[0], &indices[1], &indices[2]);
								std::vector<glm::vec3> mesh_verts;
								std::vector<glm::vec2> mesh_uvs;
								std::vector<glm::vec3> mesh_normals;
								std::vector<unsigned int> mesh_indices;
								for (int i = 0; i < 3; i++)
								{
									indices[i]--;
 									mesh_verts.push_back(vertices[indices[i]]);
									glm::vec3 normal = glm::normalize(glm::cross((vertices[indices[(i) % 3]] - vertices[indices[(i + 1) % 3]]), (vertices[indices[(i) % 3]] - vertices[indices[(i + 2) % 3]])));
									mesh_normals.push_back(normal);
									mesh_indices.push_back(i);
								}
								mesh_uvs.push_back({ 0.0f, 0.0f });
								mesh_uvs.push_back({ 1.0f, 0.0f });
								mesh_uvs.push_back({ 0.0f, 1.0f });
								mesh = new Mesh(mesh_verts, mesh_normals, mesh_uvs, std::vector<glm::vec3>(), mesh_indices);
							}
							else if (std::string(object_prop->Value()).compare(TRANSFORMS) == 0)
							{
								transform = CalculateTransforms(translations, rotations, scalings, object_prop);
							}
							object_prop = object_prop->NextSibling();
						}
						SceneObject* scene_obj = new SceneObject(std::make_shared<Mesh>(*mesh), name, glm::vec3(), glm::vec3(), glm::vec3(1.0,1.0,1.0), SHAPE_T::triangle);
						scene_obj->SetMaterial(materials[mat_ind]);
						scene_obj->SetTransforms(transform);
						//CH_TRACE(glm::to_string(scene_obj->GetMaterial()->diffuse));
						scene->AddSceneObject(scene_obj->GetName(), std::make_shared<SceneObject>(*scene_obj));
					}
					else if (std::string(child_node->Value()).compare(SPHR) == 0)
					{
						std:: string name = "sphere_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
						SceneObject* scene_obj; //= new SceneObject(std::make_shared<Mesh>(), name, glm::vec3(), glm::vec3(), glm::vec3(), SHAPE_T::sphere);
						glm::mat4 transform = glm::mat4(1.0f);
						Sphere s(NULL, true);

						tinyxml2::XMLNode* object_prop = child_node->FirstChild();
						while (object_prop)
						{
							if (std::string(object_prop->Value()).compare(MAT) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								int ind;
								sscanf(data.c_str(), "%d", &ind);
								ind = ind - 1;
								s.m_material = materials[ind];
								//scene_obj->SetMaterial(materials[ind]);
							}
							else if (std::string(object_prop->Value()).compare("Center") == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								int ind;
								sscanf(data.c_str(), "%d", &ind);
								ind = ind - 1;
								//scene_obj->SetPosition(vertices[ind]);
								//((Sphere*)(scene_obj->m_mesh->m_shapes[0].get()))->m_center = vertices[ind];
								s.m_center = vertices[ind];
							}
							else if (std::string(object_prop->Value()).compare(RAD) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								float r;
								sscanf(data.c_str(), "%f", &r);
								//scene_obj->SetScale(glm::vec3(1,1,1));
								//((Sphere*)(scene_obj->m_mesh->m_shapes[0].get()))->m_radius = r;
								s.m_radius = r;
							}
							else if (std::string(object_prop->Value()).compare(TRANSFORMS) == 0)
							{
								transform = CalculateTransforms(translations, rotations, scalings, object_prop);
							}
							object_prop = object_prop->NextSibling();
						}
						scene_obj = SceneObject::ConstructSphere(name, s, glm::vec3(), glm::vec3(), glm::vec3());
						scene_obj->SetTransforms(transform);
						//CH_TRACE(glm::to_string(scene_obj->GetMaterial()->diffuse));
						scene->AddSceneObject(scene_obj->GetName(), std::make_shared<SceneObject>(*scene_obj));
					}
					child_node = child_node->NextSibling();
				}
			}
			node = node->NextSibling();
		}
		return scene;
	}
}