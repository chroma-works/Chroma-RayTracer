#include "AssetImporter.h"

#include <iostream>
#include <sstream>
#include <string> 

#include <ray-tracer/editor/Logger.h>
#include <ray-tracer/main/Shape.h>
#include <ray-tracer/main/Material.h>
#include <ray-tracer/main/ObjectLight.h>

#include <thirdparty/hapPLY/happly.h>
#include <thirdparty/OBJ_loader/OBJ_Loader.h>
#include <thirdparty/glm/glm/gtx/quaternion.hpp>
#include <ray-tracer/main/ImageTextureMap.h>
#include <ray-tracer/main/NoiseTextureMap.h>
#include <ray-tracer/main/ProceduralTextureMap.h>


namespace CHR
{
	const std::string ABS_COEF = "AbsorptionCoefficient";
	const std::string ABS_IND = "AbsorptionIndex";
	const std::string AM_LIG = "AmbientLight";
	const std::string AM_REF = "AmbientReflectance";
	const std::string APERTURE = "ApertureSize";
	const std::string A_LIG = "AreaLight";
	const std::string BCK_COLOR = "BackgroundColor";
	const std::string BUMP_F = "BumpFactor";
	const std::string BRDFS = "BRDFs";
	const std::string CAMS = "Cameras";
	const std::string COMP = "Composite";
	const std::string CNTR = "Center";
	const std::string DEC_M = "DecalMode";
	const std::string DIF_REF = "DiffuseReflectance";
	const std::string D_LIG = "DirectionalLight";
	const std::string DIR = "Direction";
	const std::string E_LIG = "SphericalDirectionalLight";
	const std::string FACES = "Faces";
	const std::string FOCUS = "FocusDistance";
	const std::string GAZE = "Gaze";
	const std::string I_TEST_EPS = "IntersectionTestEpsilon";
	const std::string IMG = "Image";
	const std::string IMGS = "Images";
	const std::string IM_ID = "ImageId";
	const std::string IMG_NAME = "ImageName";
	const std::string IND = "Indices";
	const std::string INTERP = "Interpolation";
	const std::string INTEN = "Intensity";
	const std::string LIGS = "Lights";
	const std::string L_SPHR = "LightSphere";
	const std::string L_MESH = "LightMesh";
	const std::string MAT = "Material";
	const std::string MATS = "Materials";
	const std::string MAX_RECUR = "MaxRecursionDepth";
	const std::string MESH = "Mesh";
	const std::string MESH_INS = "MeshInstance";
	const std::string MIRROR_REF = "MirrorReflectance";
	const std::string MOTION_B = "MotionBlur";
	const std::string MOD_BLPH = "ModifiedBlinnPhong";
	const std::string MOD_PH = "ModifiedPhong";
	const std::string N_DIST = "NearDistance";
	const std::string NOISE_CONV = "NoiseConversion";
	const std::string NOISE_S = "NoiseScale";
	const std::string NORMALIZER = "Normalizer";
	const std::string N_PLANE = "NearPlane";
	const std::string NUM_SAMP = "NumSamples";
	const std::string OBJ = "Objects";
	const std::string OG_BLPH= "OriginalBlinnPhong";
	const std::string OG_PH = "OriginalPhong";
	const std::string UP = "Up";
	const std::string P_LIG = "PointLight";
	const std::string PHONG_EX = "PhongExponent";
	const std::string POS = "Position";
	const std::string RAD = "Radius";
	const std::string REF_IND = "RefractionIndex";
	const std::string RES = "ImageResolution";
	const std::string ROT = "Rotation";
	const std::string ROUGH = "Roughness";
	const std::string SHADING_M = "shadingMode";
	const std::string SMOOTH = "smooth";
	const std::string S_RAY_EPS = "ShadowRayEpsilon";
	const std::string S_LIG = "SpotLight";
	const std::string SPEC_REF = "SpecularReflectance";
	const std::string SPHR = "Sphere";
	const std::string TRIANGLE = "Triangle";
	const std::string TRANSFORMS = "Transformations";
	const std::string TRA = "Translation";
	const std::string TEX = "Textures";
	const std::string TEX_DATA = "TexCoordData";
	const std::string TEX_MAP = "TextureMap";
	const std::string TOR_SPA = "TorranceSparrow";
	const std::string SCA = "Scaling";
	const std::string VRTX_DATA = "VertexData";

	//========================================================================================================================//

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
                mesh->m_vertex_positions.push_back(std::make_shared<glm::vec3>(
					glm::eulerAngleYXZ(r.y, r.x, r.z) * glm::vec4(glm::vec3(curMesh.Vertices[j].Position.X,
                    curMesh.Vertices[j].Position.Y, curMesh.Vertices[j].Position.Z )*s + t, 1.0f)) );
                mesh->m_vertex_normals.push_back(std::make_shared < glm::vec3>(glm::eulerAngleYXZ(r.y, r.x, r.z) * glm::vec4(curMesh.Vertices[j].Normal.X,
                    curMesh.Vertices[j].Normal.Y,
                    curMesh.Vertices[j].Normal.Z,1)));
                mesh->m_vertex_texcoords.push_back(std::make_shared<glm::vec2>(curMesh.Vertices[j].TextureCoordinate.X,
                    curMesh.Vertices[j].TextureCoordinate.Y ));
            }

            mesh->m_indices = curMesh.Indices;
			mesh->SetFaceCount(mesh->m_indices.size() / 3);
			mesh->m_shading_mode = SHADING_MODE::smooth;

        }

        return mesh;
    }

	//========================================================================================================================//

    Texture* AssetImporter::LoadTexture(const std::string& file_name)
    {
        return new Texture(file_name);
    }

	//========================================================================================================================//

	std::vector<std::shared_ptr<TextureMap>> ParseTextures(tinyxml2::XMLNode* node, std::string file_path, std::vector<std::shared_ptr<Texture>>& textures)
	{
		std::vector<std::shared_ptr<TextureMap>> texture_maps;

		while (node)
		{
			if (std::string(node->Value()).compare(IMGS) == 0)
			{
				tinyxml2::XMLNode* child_node = node->FirstChild();
				while (child_node)
				{
					textures.push_back(std::make_shared<Texture>(file_path + std::string(child_node->FirstChild()->Value())));
					child_node = child_node->NextSibling();
				}
			}
			else if (std::string(node->Value()).compare(TEX_MAP) == 0)
			{
				std::string type = node->ToElement()->FindAttribute("type")->Value();
				bool degamma = false;
				if (node->ToElement()->FindAttribute("degamma"))
					degamma = node->ToElement()->FindAttribute("degamma")->BoolValue();

				if (type.compare("image") == 0)
				{
					tinyxml2::XMLNode* child_node = node->FirstChild();
					int tex_ind = -1;
					bool interp = false;
					CHR::DECAL_M d_mode;
					int normalizer = -1;
					float bump_f = 1.0f;


					while (child_node)
					{
						if (std::string(child_node->Value()).compare(IM_ID) == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							sscanf(data.c_str(), "%d", &tex_ind);
							tex_ind--;
						}
						else if (std::string(child_node->Value()).compare(DEC_M) == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							if (data.compare("replace_kd") == 0)
								d_mode = DECAL_M::re_kd;
							else if (data.compare("replace_normal") == 0)
								d_mode = DECAL_M::re_no;
							else if (data.compare("bump_normal") == 0)
								d_mode = DECAL_M::bump;
							else if (data.compare("blend_kd") == 0)
								d_mode = DECAL_M::bl_kd;
							else if (data.compare("replace_background") == 0)
								d_mode = DECAL_M::re_bg;
							else if (data.compare("replace_all") == 0)
								d_mode = DECAL_M::re_all;

						}
						else if (std::string(child_node->Value()).compare(INTERP) == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							interp = data.compare("bilinear") == 0;
						}
						else if (std::string(child_node->Value()).compare(NORMALIZER) == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							sscanf(data.c_str(), "%d", &normalizer);
						}
						else if (std::string(child_node->Value()).compare(BUMP_F) == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							sscanf(data.c_str(), "%f", &bump_f);
						}
						child_node = child_node->NextSibling();
					}
					auto tm = std::make_shared<ImageTextureMap>(textures[tex_ind], d_mode, interp);
					if(normalizer !=-1)
						tm->SetNormalizer(normalizer);
					tm->SetBumpFactor(bump_f);
					tm->SetDegamma(degamma);
					texture_maps.push_back(tm);
				}
				else if (type.compare("perlin") == 0)
				{
					tinyxml2::XMLNode* child_node = node->FirstChild();
					bool linear_conv = false;
					CHR::DECAL_M d_mode;
					int noise_scale = -1;
					float bump_f = 1.0f;

					while (child_node)
					{
						if (std::string(child_node->Value()).compare(DEC_M) == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							if (data.compare("replace_kd") == 0)
								d_mode = DECAL_M::re_kd;
							else if (data.compare("replace_normal") == 0)
								d_mode = DECAL_M::re_no;
							else if (data.compare("bump_normal") == 0)
								d_mode = DECAL_M::bump;
							else if (data.compare("blend_kd") == 0)
								d_mode = DECAL_M::bl_kd;
							else if (data.compare("replace_background") == 0)
								d_mode = DECAL_M::re_bg;
							else if (data.compare("replace_all") == 0)
								d_mode = DECAL_M::re_all;

						}
						else if (std::string(child_node->Value()).compare(NOISE_CONV) == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							linear_conv = data.compare("linear") == 0;
						}
						else if (std::string(child_node->Value()).compare(NOISE_S) == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							sscanf(data.c_str(), "%d", &noise_scale);
						}
						else if (std::string(child_node->Value()).compare(BUMP_F) == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							sscanf(data.c_str(), "%f", &bump_f);
						}
						child_node = child_node->NextSibling();
					}
					auto tm = std::make_shared<NoiseTextureMap>(d_mode, linear_conv);
					if (noise_scale != -1)
						tm->SetScale(noise_scale);
					tm->SetBumpFactor(bump_f);
					texture_maps.push_back(tm);
				}
				else if (type.compare("checkerboard") == 0)
				{
					tinyxml2::XMLNode* child_node = node->FirstChild();
					float offset, scale;
					CHR::DECAL_M d_mode;
					glm::vec3 b, w;

					while (child_node)
					{
						if (std::string(child_node->Value()).compare(DEC_M) == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							if (data.compare("replace_kd") == 0)
								d_mode = DECAL_M::re_kd;
							else if (data.compare("replace_normal") == 0)
								d_mode = DECAL_M::re_no;
							else if (data.compare("bump_normal") == 0)
								d_mode = DECAL_M::bump;
							else if (data.compare("blend_kd") == 0)
								d_mode = DECAL_M::bl_kd;
							else if (data.compare("replace_background") == 0)
								d_mode = DECAL_M::re_bg;
							else if (data.compare("replace_all") == 0)
								d_mode = DECAL_M::re_all;

						}
						else if (std::string(child_node->Value()).compare("BlackColor") == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							sscanf(data.c_str(), "%f %f %f", &b.r, &b.g, &b.b);
						}
						else if (std::string(child_node->Value()).compare("WhiteColor") == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							sscanf(data.c_str(), "%f %f %f", &w.r, &w.g, &w.b);
						}
						else if (std::string(child_node->Value()).compare("Scale") == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							sscanf(data.c_str(), "%f", &scale);
						}
						else if (std::string(child_node->Value()).compare("Offset") == 0)
						{
							std::string data = child_node->FirstChild()->Value();
							sscanf(data.c_str(), "%f", &offset);
						}
						child_node = child_node->NextSibling();
					}
					auto tm = std::make_shared<ProcedurelTextureMap>(d_mode, b, w, scale, offset);
					texture_maps.push_back(tm);
				}
			}
			node = node->NextSibling();
		}

		return texture_maps;
	}

	//========================================================================================================================//

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

	//========================================================================================================================//

	glm::mat4 CalculateTransforms(
		std::vector<glm::mat4> translations,
		std::vector<glm::mat4> rotations, 
		std::vector<glm::mat4> scalings,
		std::vector<glm::mat4> composites,
		tinyxml2::XMLNode* node)
	{
		std::string data = node->FirstChild()->Value();
		std::vector<std::string> v;

		Split(data, v, ' ');
		glm::mat4 trnsfm = glm::mat4(1.0f);

		for (int i = v.size()-1; i >=0 ; i--)
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
			else if (v[i].at(0) == 'c')
			{
				trnsfm *= composites[std::stoi(v[i].substr(1)) - 1];
			}
		}
		return trnsfm;
	}

	//========================================================================================================================//

	Mesh* AssetImporter::LoadMeshFromPly(std::string ply_path)
	{
		std::vector<std::shared_ptr<glm::vec3>> mesh_verts;
		std::vector<std::shared_ptr<glm::vec2>> mesh_uvs;
		std::vector<std::shared_ptr<glm::vec3>> mesh_normals;
		std::vector<unsigned int> mesh_indices;

		happly::PLYData ply_in(ply_path);
		std::vector<std::array<double, 3>> v_pos = ply_in.getVertexPositions();
		std::vector<double> us;
		std::vector<double> vs;
		if (ply_in.hasElement("vertex") && 
			ply_in.getElement("vertex").hasProperty("u") &&
			ply_in.getElement("vertex").hasProperty("v"))
		{
			us = ply_in.getElement("vertex").getProperty<double>("u");
			vs = ply_in.getElement("vertex").getProperty<double>("v");

		}
		std::vector<std::vector<size_t>> f_ind = ply_in.getFaceIndices<size_t>();

		for (int i = 0; i < v_pos.size(); i++)
		{
			mesh_verts.push_back(std::make_shared<glm::vec3>(v_pos[i][0], v_pos[i][1], v_pos[i][2]));
			if(ply_in.hasElement("vertex") &&
				ply_in.getElement("vertex").hasProperty("u") &&
				ply_in.getElement("vertex").hasProperty("v"))
				mesh_uvs.push_back(std::make_shared<glm::vec2>(us[i], vs[i]));
			/*mesh_verts.push_back({ v_pos[i+1][0], v_pos[i+1][1], v_pos[i+1][2] });
			mesh_verts.push_back({ v_pos[i+2][0], v_pos[i+2][1], v_pos[i+2][2] });*/
		}
		for (auto faces : f_ind)
		{
			glm::vec3 a = (*mesh_verts[faces[1]] - *mesh_verts[faces[0]]);
			glm::vec3 b = (*mesh_verts[faces[2]] - *mesh_verts[faces[0]]);
			glm::vec3 normal = glm::normalize(glm::cross(-a, -b));
			if(faces.size() ==3)		//triangle faces
				for (int i = 0; i < faces.size(); i ++)
				{
					mesh_normals.push_back(std::make_shared<glm::vec3>(normal));
					mesh_indices.push_back(faces[i]);
				}
			else if(faces.size() == 4)	//quad faces
			{
				for (int start_ind = 1; start_ind+2 <= faces.size(); start_ind++)
				{
					mesh_normals.push_back(std::make_shared<glm::vec3>(normal));
					mesh_indices.push_back(faces[0]);
					for (int i = start_ind; i < start_ind + 2; i++)
					{
						mesh_normals.push_back(std::make_shared<glm::vec3>(normal));
						mesh_indices.push_back(faces[i]);
					}
				}
			}

		}
		return new Mesh(mesh_verts, mesh_normals, mesh_uvs, std::vector<std::shared_ptr<glm::vec3>>(), mesh_indices);
	}

	//========================================================================================================================//

	Camera* ParseCamera(tinyxml2::XMLNode* node)
	{
		Camera* cam = new Camera(1.0f * 1280,
			1.0f * 720, 0.1f, 300.0f, 60.0f); //Standart Camera(raster pipeline parameters)
		tinyxml2::XMLNode* cam_prop = node->FirstChild();

		glm::vec2 vec[2];

		auto cam_type = node->ToElement()->FindAttribute("type");
		auto handedness = node->ToElement()->FindAttribute("handedness");

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
			else if (std::string(cam_prop->Value()).compare(GAZE) == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				glm::vec3 vec({ 0,0,0 });
				sscanf(data.c_str(), "%f %f %f", &vec.x, &vec.y, &vec.z);
				cam->SetGaze(vec);
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
			else if (std::string(cam_prop->Value()).compare(IMG_NAME) == 0)
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
			else if (std::string(cam_prop->Value()).compare(FOCUS) == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				float d;
				sscanf(data.c_str(), "%f", &d);
				cam->SetFocalDistance(d);
			}
			else if (std::string(cam_prop->Value()).compare(APERTURE) == 0)
			{
				std::string data = cam_prop->FirstChild()->Value();
				float d;
				sscanf(data.c_str(), "%f", &d);
				cam->SetApertureSize(d);
			}
			//---------------Render params-----------------
			else if (std::string(cam_prop->Value()).compare("RendererParams") == 0)
			{
				std::string data;
				if(cam_prop->FirstChild())
					 data = cam_prop->FirstChild()->Value();

				std::vector<std::string> v;
				if(data.length() > 0)
					Split(data, v, ' ');

				for (auto s : v)
				{
					if (s.compare("NextEventEstimation") == 0)
						cam->SetNextEventEstimation(true);
					else if (s.compare("RussianRoulette") == 0)
						cam->SetRussianRoulette(true);
					else if (s.compare("ImportanceSampling") == 0)
						cam->SetImportanceSampling(true);
				}
			}
			//---------------Tone Map params-----------------
			else if (std::string(cam_prop->Value()).compare("Tonemap") == 0)
			{
				auto tone_prop = cam_prop->FirstChild();
				while (tone_prop)//iterate over tonemap items
				{
					auto prop_name = std::string(tone_prop->Value());
					if (prop_name.compare("TMOOptions") == 0)
					{
						std::string data = tone_prop->FirstChild()->Value();
						sscanf(data.c_str(), "%f %f", &cam->m_key_val, &cam->m_burn_perc);
					}
					else if (prop_name.compare("Saturation") == 0)
					{
						std::string data = tone_prop->FirstChild()->Value();
						sscanf(data.c_str(), "%f", &cam->m_saturation);
					}
					else if (prop_name.compare("Gamma") == 0)
					{
						std::string data = tone_prop->FirstChild()->Value();
						sscanf(data.c_str(), "%f", &cam->m_gamma);
					}
					tone_prop = tone_prop->NextSibling();
				}
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
				vec[0].y = tanf(glm::radians(fovy* 0.5f));
				vec[1].y = -vec[0].y;
				vec[0].x = vec[1].y;
				vec[1].x = -vec[0].x;
				cam->SetNearPlane(vec);
			}
			cam_prop = cam_prop->NextSibling();
		}
		if (handedness && std::string(handedness->Value()).compare("left")==0)
		{
			cam->m_left_handed = true;
		}
		if (glm::dot(cam->GetGaze(), cam->GetUp()) != 0)
		{
			CH_TRACE("Camera Up and gaze are not orthogonal. Recalculating up...");
			glm::vec3 u = glm::cross(cam->GetGaze(), cam->GetUp());
			glm::vec3 v_prime = glm::cross(-cam->GetGaze(), u);
			cam->SetUp(v_prime);
		}
		//fix axpect ratio
		if (cam_type && std::string(cam_type->Value()).compare("lookAt") == 0)
		{
			glm::ivec2 res = cam->GetResolution();
			float aspect_ratio = res.x/((float)res.y);
			vec[0].x = vec[1].y * aspect_ratio;
			vec[1].x = -vec[0].x;
			cam->SetNearPlane(vec);
		}
		
		return cam;
	}

	//========================================================================================================================//

	std::vector<std::shared_ptr<glm::vec3>> ParseVertexData(tinyxml2::XMLNode* node)
	{
		std::vector<std::shared_ptr<glm::vec3>> vertices;
		std::string data = node->Value();
		std::string line;
		std::istringstream stream(data);

		while (std::getline(stream, line)) {//read vertices line by line
			glm::vec3 vertex;
			std::istringstream iss(line);
			iss >> vertex.x >> vertex.y >> vertex.z;

			if (iss)
				vertices.push_back(std::make_shared<glm::vec3>(vertex));
		}

		return vertices;
	}

	//========================================================================================================================//

	std::vector<std::shared_ptr<glm::vec2>> ParseTextureCoords(tinyxml2::XMLNode* node)
	{
		std::vector<std::shared_ptr<glm::vec2>> tex_coords;
		std::string data = node->Value();
		std::string line;
		std::istringstream stream(data);

		while (std::getline(stream, line)) {//read vertices line by line
			glm::vec2 tex_coord;
			std::istringstream iss(line);
			iss >> tex_coord.x >> tex_coord.y;

			if (iss)
				tex_coords.push_back(std::make_shared<glm::vec2>(tex_coord));
		}

		return tex_coords;
	}

	//========================================================================================================================//

	Scene* AssetImporter::LoadSceneFromXML(Shader* shader, const std::string& file_path)
	{
		//Get Settings pointer
		auto settings = Settings::GetInstance();
		//Scene name from file name
		size_t found = file_path.find_last_of("/\\");
		size_t found2 = file_path.find_last_of(".");
		std::string file_name = file_path.substr(found+1, found2-found-1);

		Scene* scene = new Scene(file_name, shader);

		tinyxml2::XMLDocument doc;
		doc.LoadFile(file_path.c_str());
		std::vector<std::shared_ptr<BRDF>> brdfs;
		std::vector<std::shared_ptr<Material>> materials;
		std::vector<std::shared_ptr<TextureMap>> texturemaps;
		std::vector<std::shared_ptr<Texture>> textures;		//just incase of a env. light
		std::vector<std::shared_ptr<glm::vec3>> vertices;
		std::vector<std::shared_ptr<glm::vec2>>texture_coords;

		std::vector<glm::mat4> translations;
		std::vector<glm::mat4> rotations;
		std::vector<glm::mat4> scalings;
		std::vector<glm::mat4> composites;

		tinyxml2::XMLNode* node = doc.RootElement()->FirstChild();
		while (node)
		{
			if (std::string(node->Value()).compare(MAX_RECUR) == 0)
			{
				std::string data = node->FirstChild()->Value();
				sscanf(data.c_str(), "%d", &settings->m_recur_depth);
			}
			else if (std::string(node->Value()).compare(BCK_COLOR) == 0)
			{
				std::string data = node->FirstChild()->Value();
				sscanf(data.c_str(), "%f %f %f", &scene->m_sky_color.x, &scene->m_sky_color.y, &scene->m_sky_color.z);
			}
			else if (std::string(node->Value()).compare(S_RAY_EPS) == 0)
			{
				std::string data = node->FirstChild()->Value();
				sscanf(data.c_str(), "%f", &settings->m_shadow_eps);
			}
			else if (std::string(node->Value()).compare(I_TEST_EPS) == 0)
			{
				std::string data = node->FirstChild()->Value();
				sscanf(data.c_str(), "%f", &settings->m_intersection_eps);
			}
			else if (std::string(node->Value()).compare(CAMS) == 0)
			{
				tinyxml2::XMLNode* child_node = node->FirstChild();
				while (child_node)//iterate over cameras
				{
					std::string cam_name = "camera_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
					Camera* cam = ParseCamera( child_node);

					scene->AddCamera(cam_name, cam);
					scene->GetCamera(cam_name)->SetName(cam_name);

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
								sscanf(data.c_str(), "%f %f %f", &p_l.m_position.x, &p_l.m_position.y, &p_l.m_position.z);
							}
							else if (std::string(lig_prop->Value()).compare(INTEN) == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								glm::vec3 tmp({ 10,10,10 });
								sscanf(data.c_str(), "%f %f %f", &tmp.x, &tmp.y, &tmp.z);

								p_l.SetIntensity(tmp);
								p_l.m_ambient = glm::clamp(scene->m_ambient_l / 1000.0f, 0.0f, 1.0f);//Reset ambient light to approixmate ray traced env
							}
							lig_prop = lig_prop->NextSibling();
						}
						scene->AddLight(lig_name, std::make_shared<PointLight>(p_l));
						scene->m_lights[lig_name]->SetName(lig_name);

					}
					else if (std::string(child_node->Value()).compare(D_LIG) == 0)
					{
						DirectionalLight d_l = DirectionalLight({ 0,0,0 }, { 0,0,0 }, { 0,0,0 }, { 0,0,0 });
						std::string lig_name;
						lig_name = "directional_light_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
						tinyxml2::XMLNode* lig_prop = child_node->FirstChild();

						while (lig_prop)//iterate over each light properties
						{
							if (std::string(lig_prop->Value()).compare(DIR) == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f %f %f", &d_l.m_direction.x, &d_l.m_direction.y, &d_l.m_direction.z);
							}
							else if (std::string(lig_prop->Value()).compare("Radiance") == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								glm::vec3 tmp({ 10,10,10 });
								sscanf(data.c_str(), "%f %f %f", &tmp.x, &tmp.y, &tmp.z);

								d_l.SetIntensity(tmp);
								d_l.m_ambient = glm::clamp(scene->m_ambient_l / 1000.0f, 0.0f, 1.0f);//Reset ambient light to approixmate ray traced env
							}
							lig_prop = lig_prop->NextSibling();
						}
						scene->AddLight(lig_name, std::make_shared<DirectionalLight>(d_l));
						scene->m_lights[lig_name]->SetName(lig_name);
					}
					else if (std::string(child_node->Value()).compare(S_LIG) == 0)
					{
						SpotLight s_l = SpotLight({ 0,0,0 }, { 0,0,0 }, { 0,0,0 }, { 0,0,0 }, { 0,0,0 });
						std::string lig_name;
						lig_name = "spot_light_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
						tinyxml2::XMLNode* lig_prop = child_node->FirstChild();

						while (lig_prop)//iterate over each light properties
						{
							if (std::string(lig_prop->Value()).compare(POS) == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f %f %f", &s_l.m_position.x, &s_l.m_position.y, &s_l.m_position.z);
							}
							else if (std::string(lig_prop->Value()).compare(DIR) == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f %f %f", &s_l.m_direction.x, &s_l.m_direction.y, &s_l.m_direction.z);
							}
							else if (std::string(lig_prop->Value()).compare(INTEN) == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								glm::vec3 tmp({ 10,10,10 });
								sscanf(data.c_str(), "%f %f %f", &tmp.x, &tmp.y, &tmp.z);

								s_l.SetIntensity(tmp);
								s_l.m_ambient = glm::clamp(scene->m_ambient_l / 1000.0f, 0.0f, 1.0f);//Reset ambient light to approixmate ray traced env
							}
							else if (std::string(lig_prop->Value()).compare("CoverageAngle") == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f", &s_l.m_cut_off);
								s_l.m_cut_off = glm::radians(s_l.m_cut_off);
							}
							else if (std::string(lig_prop->Value()).compare("FalloffAngle") == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f", &s_l.m_fall_off);
								s_l.m_fall_off = glm::radians(s_l.m_fall_off);
							}
							lig_prop = lig_prop->NextSibling();
						}
						scene->AddLight(lig_name, std::make_shared<SpotLight>(s_l));
						scene->m_lights[lig_name]->SetName(lig_name);
					}
					else if (std::string(child_node->Value()).compare(A_LIG) == 0)
					{
						AreaLight a_l = AreaLight({ 0,0,0 }, { 0,0,0 }, 0.0f);
						std::string lig_name;
						lig_name = "area_light_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
						tinyxml2::XMLNode* lig_prop = child_node->FirstChild();

						while (lig_prop)//iterate over each light properties
						{
							if (std::string(lig_prop->Value()).compare(POS) == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f %f %f", &a_l.m_position.x, &a_l.m_position.y, &a_l.m_position.z);
							}
							else if (std::string(lig_prop->Value()).compare("Normal") == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f %f %f", &a_l.m_normal.x, &a_l.m_normal.y, &a_l.m_normal.z);
								a_l.m_normal = glm::normalize(a_l.m_normal);
							}
							else if (std::string(lig_prop->Value()).compare("Radiance") == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								glm::vec3 tmp({ 10,10,10 });
								sscanf(data.c_str(), "%f %f %f", &tmp.x, &tmp.y, &tmp.z);

								a_l.SetIntensity(tmp);
								a_l.m_ambient = glm::clamp(scene->m_ambient_l / 1000.0f, 0.0f, 1.0f);//Reset ambient light to approixmate ray traced env
							}
							else if (std::string(lig_prop->Value()).compare("Size") == 0)
							{
								std::string data = lig_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f", &a_l.m_size);
							}
							lig_prop = lig_prop->NextSibling();
						}
						scene->AddLight(lig_name, std::make_shared<AreaLight>(a_l));
						scene->m_lights[lig_name]->SetName(lig_name);
					}
					else if (std::string(child_node->Value()).compare(E_LIG) == 0)
					{
						std::string lig_name;
						lig_name = "environment_light_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
						tinyxml2::XMLNode* lig_prop = child_node->FirstChild();
						auto e_l = std::make_shared<EnvironmentLight>(textures[atoi(lig_prop->FirstChild()->Value()) - 1]);
						e_l->SetName(lig_name);

						scene->AddLight(lig_name, e_l);
						scene->m_lights[lig_name]->SetName(lig_name);
						scene->m_sky_texture = std::make_shared<ImageTextureMap>(
							textures[atoi(lig_prop->FirstChild()->Value()) - 1], DECAL_M::bl_kd);
						scene->m_map_texture_to_sphere = true;
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
					else if (std::string(child_node->Value()).compare(COMP) == 0)
					{
						std::string data = child_node->FirstChild()->Value();
						std::istringstream iss(data);
						glm::mat4 mat;
						int i = 0;
						double n;
						while (iss >> n)
						{
							mat[i%4][i/4] = n;
							i++;
						}
						composites.push_back(mat);
					}
					child_node = child_node->NextSibling();
				}
			}
			else if (std::string(node->Value()).compare(BRDFS) == 0)
			{
			tinyxml2::XMLNode* child_node = node->FirstChild();
				while (child_node)//iterate over BRDFS
				{
					std::string data = child_node->FirstChild()->FirstChild()->Value();
					float exp = 50.0f;
					sscanf(data.c_str(), "%f", &exp);

					bool normalized = false;
					if (child_node->ToElement()->FindAttribute("normalized"))
						normalized = child_node->ToElement()->FindAttribute("normalized")->BoolValue();
					if (std::string(child_node->Value()).compare(OG_BLPH) == 0)
					{
						auto brdf = std::make_shared<BlinnPhong>(exp, normalized);
						brdfs.push_back(brdf);
					}
					else if (std::string(child_node->Value()).compare(OG_PH) == 0)
					{
						auto brdf = std::make_shared<Phong>(exp, normalized);
						brdfs.push_back(brdf);
					}
					else if (std::string(child_node->Value()).compare(MOD_BLPH) == 0)
					{
						auto brdf = std::make_shared<ModifiedBlinnPhong>(exp, normalized);
						brdfs.push_back(brdf);
					}
					else if (std::string(child_node->Value()).compare(MOD_PH) == 0)
					{
						auto brdf = std::make_shared<ModifiedPhong>(exp, normalized);
						brdfs.push_back(brdf);
					}
					else if (std::string(child_node->Value()).compare(TOR_SPA) == 0)
					{
						bool kd_f = child_node->ToElement()->FindAttribute("kdfresnel")->BoolValue();
						auto brdf = std::make_shared<TorranceSparrow>(kd_f, exp, normalized);
						brdfs.push_back(brdf);
					}
					child_node = child_node->NextSibling();
				}
			}
			else if (std::string(node->Value()).compare(MATS) == 0)
			{
				tinyxml2::XMLNode* child_node = node->FirstChild();
				while (child_node)//iterate over Materials
				{
					std::shared_ptr<Material> mat;
					MAT_TYPE type = MAT_TYPE::none;

					//torrance sparrow indices
					float ts_ref_ind = NAN;
					float ts_abs_ind = NAN;

					bool flag = true;
					auto nn = child_node->ToElement()->FindAttribute("degamma");
					bool degamma = false;
					if(nn)
						degamma= std::string(nn->Value()).compare("true") == 0;
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
									mat = std::make_shared<Mirror>(); flag = false;
								if (std::string(material_prop->Value()).compare(MIRROR_REF) == 0)
								{
									std::string data = material_prop->FirstChild()->Value();
									sscanf(data.c_str(), "%f %f %f", &(static_cast<Mirror*>(mat.get()))->m_mirror_reflec.x,
										&(static_cast<Mirror*>(mat.get()))->m_mirror_reflec.y,
										&(static_cast<Mirror*>(mat.get()))->m_mirror_reflec.z);
								}
							}
								break;
							case MAT_TYPE::conductor:
							{
								if (flag)
									mat = std::make_shared<Conductor>(); flag = false;
								if (std::string(material_prop->Value()).compare(MIRROR_REF) == 0)
								{
									std::string data = material_prop->FirstChild()->Value();
									sscanf(data.c_str(), "%f %f %f", &(static_cast<Conductor*>(mat.get()))->m_mirror_reflec.x,
										&(static_cast<Conductor*>(mat.get()))->m_mirror_reflec.y,
										&(static_cast<Conductor*>(mat.get()))->m_mirror_reflec.z);
								}
								else if (std::string(material_prop->Value()).compare(REF_IND) == 0)
								{
									std::string data = material_prop->FirstChild()->Value();
									sscanf(data.c_str(), "%f", &(static_cast<Conductor*>(mat.get()))->m_refraction_ind);
								}
								else if (std::string(material_prop->Value()).compare(ABS_IND) == 0)
								{
									std::string data = material_prop->FirstChild()->Value();
									sscanf(data.c_str(), "%f", &(static_cast<Conductor*>(mat.get()))->m_absorption_ind);
								}
							}
								break;
							case MAT_TYPE::dielectric:
							{
								if (flag)
									mat = std::make_shared<Dielectric>(); flag = false;
								if (std::string(material_prop->Value()).compare(REF_IND) == 0)
								{
									std::string data = material_prop->FirstChild()->Value();
									sscanf(data.c_str(), "%f", &(static_cast<Dielectric*>(mat.get()))->m_refraction_ind);
								}
								else if (std::string(material_prop->Value()).compare(ABS_COEF) == 0)
								{
									std::string data = material_prop->FirstChild()->Value();
									sscanf(data.c_str(), "%f %f %f", &(static_cast<Dielectric*>(mat.get()))->m_absorption_coeff.x,
										&(static_cast<Dielectric*>(mat.get()))->m_absorption_coeff.y,
										&(static_cast<Dielectric*>(mat.get()))->m_absorption_coeff.z);
								}
							}
								break;
							default:
							{
								if (flag)
									mat = std::make_shared<Material>(); flag = false;
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
							sscanf(data.c_str(), "%f", &mat->m_brdf->m_exponent);
						}
						else if (std::string(material_prop->Value()).compare(ROUGH) == 0)
						{
							std::string data = material_prop->FirstChild()->Value();
							sscanf(data.c_str(), "%f", &mat->m_roughness);
						}
						else if (std::string(material_prop->Value()).compare(REF_IND) == 0)//Torrance Sparrow BRDF
						{
							std::string data = material_prop->FirstChild()->Value();
							sscanf(data.c_str(), "%f", &ts_ref_ind);
						}
						else if (std::string(material_prop->Value()).compare(ABS_IND) == 0)//Torrance Sparrow BRDF
						{
							std::string data = material_prop->FirstChild()->Value();
							sscanf(data.c_str(), "%f", &ts_abs_ind);
						}

						material_prop = material_prop->NextSibling();
					}

					if (child_node->ToElement()->FindAttribute("BRDF"))
					{
						mat->m_brdf = brdfs[child_node->ToElement()->FindAttribute("BRDF")->IntValue()-1];
						if (mat->m_brdf->m_type == BRDF_T::tor_spa)
						{
							auto ptr = std::dynamic_pointer_cast<TorranceSparrow>(mat->m_brdf);
							ptr->SetFresnelCoeffs(ts_ref_ind, ts_abs_ind);
						}
					}
					child_node = child_node->NextSibling();
					if(degamma)
						mat->Degamma();
					materials.push_back(mat);
				}
			}
			else if (std::string(node->Value()).compare(TEX) == 0)
			{
				tinyxml2::XMLNode* child_node = node->FirstChild();

				texturemaps = ParseTextures(child_node, file_path.substr(0, found + 1), textures);
				for (auto t : texturemaps)
				{
					if (t->GetDecalMode() == DECAL_M::re_bg)
					{
						scene->m_sky_texture = t;
						break;
					}
				}
			}
			else if (std::string(node->Value()).compare(VRTX_DATA) == 0)
			{
				tinyxml2::XMLNode* child_node = node->FirstChild();
				//CH_TRACE("Parsing vertices");
				vertices = ParseVertexData(child_node);
			}
			else if (std::string(node->Value()).compare(TEX_DATA) == 0)
			{
				tinyxml2::XMLNode* child_node = node->FirstChild();
				texture_coords = ParseTextureCoords(child_node);
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
						std::shared_ptr<Mesh> mesh = nullptr;
						int mat_ind = 0, tex_map_ind_1 = -1, tex_map_ind_2 = -1;
						auto shading_mode = child_node->ToElement()->FindAttribute(SHADING_M.c_str());
						bool smooth_normals = (shading_mode ? (std::string(shading_mode->Value()).compare(SMOOTH) == 0 ? 
							true : false) : false);
							std::string().compare(SMOOTH) == 0;
						glm::mat4 transform = glm::mat4(1.0f);

						glm::vec3 m_b = {0,0,0};

						bool has_tex = false;
						std::vector<unsigned int> tex_inds(0);

						while (object_prop)
						{
							if (std::string(object_prop->Value()).compare(MAT) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%d", &mat_ind);
								mat_ind --;
							}
							else if (std::string(object_prop->Value()).compare(TEX) == 0)
							{
								has_tex = true;
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%d %d", &tex_map_ind_1,  &tex_map_ind_2);
								tex_map_ind_1--; tex_map_ind_2--;;
							}
							else if (std::string(object_prop->Value()).compare(FACES) == 0)
							{
								auto ply_file_path = object_prop->ToElement()->FindAttribute("plyFile");
								if (ply_file_path)
								{
									mesh = std::shared_ptr<Mesh>(LoadMeshFromPly(file_path.substr(0, found + 1) + std::string(ply_file_path->Value())));
									//mesh = new Mesh(mesh_verts, mesh_normals, mesh_uvs, std::vector<glm::vec3>(), mesh_indices);
								}
								else
								{
									std::string data = object_prop->FirstChild()->Value();
									std::vector<std::shared_ptr<glm::vec3>> mesh_verts;
									std::vector<std::shared_ptr<glm::vec2>> mesh_uvs;
									std::vector<std::shared_ptr<glm::vec3>> mesh_normals;
									std::vector<unsigned int> mesh_indices;

									int t_off = 0, v_off = 0;

									auto vertex_offset = object_prop->ToElement()->FindAttribute("vertexOffset");
									auto texture_offset = object_prop->ToElement()->FindAttribute("textureOffset");

									if (vertex_offset != NULL)
										v_off = vertex_offset->IntValue();

									if (texture_offset != NULL)
										t_off = texture_offset->IntValue();

									std::string line;
									std::istringstream stream(data);
									mesh_verts = vertices;
									if (has_tex)
									{
										mesh_uvs = texture_coords;
									}
									mesh_normals.reserve(vertices.size());
									mesh_normals.resize(vertices.size());
									

									while (std::getline(stream, line)) //read faces line by line
									{
										unsigned int ind[3];
										std::istringstream iss(line);
										iss >> ind[0] >> ind[1] >> ind[2];

										if (iss) {

											glm::vec3 a = (*vertices[ind[0] - 1 + v_off] - *vertices[ind[1] - 1 + v_off]);
											glm::vec3 b = (*vertices[ind[0] - 1 + v_off] - *vertices[ind[2] - 1 + v_off]);

											glm::vec3 normal = glm::vec3(0.0f, 0.0f, 0.0f);//glm::normalize(glm::cross(a, b));//calculate face normal

											normal = (glm::cross(-a, -b));

											for (int j = 0; j < 3; j++)
											{
												mesh_indices.push_back(ind[j] - 1 + v_off);
												mesh_normals[ind[j] - 1 + v_off] = std::make_shared<glm::vec3>(glm::normalize(normal));
												if (has_tex)
												{
													if (v_off != 0)
													{
														tex_inds.push_back(ind[j]-1 + t_off);
													}
												}
											}
										}
									}
									
									mesh = std::shared_ptr<Mesh>
										(new Mesh(mesh_verts, mesh_normals, mesh_uvs, std::vector<std::shared_ptr<glm::vec3>>(), mesh_indices));
								}

							}
							else if (std::string(object_prop->Value()).compare(TRANSFORMS) == 0)
							{
								transform = CalculateTransforms(translations, rotations,scalings, composites, object_prop);
							}
							else if (std::string(object_prop->Value()).compare(MOTION_B) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f %f %f", &m_b.x, &m_b.y, &m_b.z);
							}
							object_prop = object_prop->NextSibling();
						}
						auto scene_obj = std::shared_ptr<SceneObject>
							(new SceneObject(mesh, name, glm::vec3(), glm::vec3(), glm::vec3(1.0, 1.0, 1.0), SHAPE_T::triangle, tex_inds));
						scene_obj->SetMaterial(materials[mat_ind]);
						scene_obj->SetTransforms(transform);
						scene_obj->SetMotionBlur(m_b);
						if (tex_map_ind_1 > -1)
							scene_obj->SetTextureMap(texturemaps[tex_map_ind_1]);
						if (tex_map_ind_2 > -1)
							scene_obj->SetTextureMap(texturemaps[tex_map_ind_2]);

						if (smooth_normals)
						{
							scene_obj->SmoothNormals();
						}
						scene->AddSceneObject(scene_obj->GetName(), scene_obj);
					}
					else if (std::string(child_node->Value()).compare(TRIANGLE) == 0)
					{
						tinyxml2::XMLNode* object_prop = child_node->FirstChild();
						std::string name = "triangle_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
						std::shared_ptr<Mesh> mesh = nullptr;
						int mat_ind = 0, tex_map_ind_1 = -1, tex_map_ind_2 = -1;
						glm::mat4 transform = glm::mat4(1.0f);

						glm::vec3 m_b = { 0,0,0 };

						while (object_prop)
						{
							if (std::string(object_prop->Value()).compare(MAT) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%d", &mat_ind);
								mat_ind = mat_ind - 1;
							}
							else if (std::string(object_prop->Value()).compare(TEX) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%d %d", &tex_map_ind_1,  &tex_map_ind_2);
								tex_map_ind_1--; 
								tex_map_ind_2--;
							}
							else if (std::string(object_prop->Value()).compare(IND) == 0)
							{
								int indices[3];
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%d %d %d", &indices[0], &indices[1], &indices[2]);
								std::vector<std::shared_ptr<glm::vec3>> mesh_verts;
								std::vector<std::shared_ptr<glm::vec2>> mesh_uvs;
								std::vector<std::shared_ptr<glm::vec3>> mesh_normals;
								std::vector<unsigned int> mesh_indices;
								for (int i = 0; i < 3; i++)
								{
									indices[i]--;
 									mesh_verts.push_back(vertices[indices[i]]);
									glm::vec3 normal = glm::normalize(glm::cross((*vertices[indices[(i) % 3]] - *vertices[indices[(i + 1) % 3]]), 
										(*vertices[indices[(i) % 3]] - *vertices[indices[(i + 2) % 3]])));
									mesh_normals.push_back(std::make_shared<glm::vec3>(normal));
									mesh_indices.push_back(i);
								}
								mesh_uvs.push_back(std::make_shared<glm::vec2>( 0.0f, 0.0f ));
								mesh_uvs.push_back(std::make_shared<glm::vec2>(1.0f, 0.0f));
								mesh_uvs.push_back(std::make_shared<glm::vec2>(0.0f, 1.0f));
								mesh = std::shared_ptr<Mesh>
									(new Mesh(mesh_verts, mesh_normals, mesh_uvs, std::vector<std::shared_ptr<glm::vec3>>(), mesh_indices));
							}
							else if (std::string(object_prop->Value()).compare(TRANSFORMS) == 0)
							{
								transform = CalculateTransforms(translations, rotations, scalings, composites, object_prop);
							}
							else if (std::string(object_prop->Value()).compare(MOTION_B) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f %f %f", &m_b.x, &m_b.y, &m_b.z);
							}
							object_prop = object_prop->NextSibling();
						}
						auto scene_obj = std::shared_ptr<SceneObject>
							(new SceneObject(mesh, name, glm::vec3(), 
								glm::vec3(), glm::vec3(1.0,1.0,1.0), SHAPE_T::triangle));
						scene_obj->SetMaterial(materials[mat_ind]);
						scene_obj->SetTransforms(transform);
						scene_obj->SetMotionBlur(m_b); 
						if (tex_map_ind_1 > -1)
							scene_obj->SetTextureMap(texturemaps[tex_map_ind_1]);
						if (tex_map_ind_2 > -1)
							scene_obj->SetTextureMap(texturemaps[tex_map_ind_2]);
						//CH_TRACE(glm::to_string(scene_obj->GetMaterial()->diffuse));
						scene->AddSceneObject(scene_obj->GetName(), scene_obj);
					}
					else if (std::string(child_node->Value()).compare(SPHR) == 0)
					{
						std:: string name = "sphere_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
						glm::mat4 transform = glm::mat4(1.0f);
						auto s = std::make_shared<Sphere>(nullptr, true);

						glm::vec3 m_b = { 0,0,0 };
						glm::vec3 center = { 0,0,0 };
						glm::vec3 rad_scale = { 1,1,1 };

						int tex_map_ind_1 = -1, tex_map_ind_2 = -1;

						tinyxml2::XMLNode* object_prop = child_node->FirstChild();
						while (object_prop)
						{
							if (std::string(object_prop->Value()).compare(MAT) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								int ind;
								sscanf(data.c_str(), "%d", &ind);
								ind = ind - 1;
								s->m_material = materials[ind];
							}
							else if (std::string(object_prop->Value()).compare(TEX) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%d %d", &tex_map_ind_1,  &tex_map_ind_2);
								tex_map_ind_1--; 
								tex_map_ind_2--;
							}
							else if (std::string(object_prop->Value()).compare("Center") == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								int ind;
								sscanf(data.c_str(), "%d", &ind);
								ind = ind - 1;
								center = *vertices[ind];
							}
							else if (std::string(object_prop->Value()).compare(RAD) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								float r;
								sscanf(data.c_str(), "%f", &r);
								rad_scale = r * rad_scale;
							}
							else if (std::string(object_prop->Value()).compare(TRANSFORMS) == 0)
							{
								transform = CalculateTransforms(translations, rotations, scalings, composites, object_prop);
							}
							else if (std::string(object_prop->Value()).compare(MOTION_B) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f %f %f", &m_b.x, &m_b.y, &m_b.z);
							}
							object_prop = object_prop->NextSibling();
						}
						transform *= glm::scale(glm::translate(glm::mat4(1.0f), center), rad_scale);
						auto scene_obj = std::shared_ptr<SceneObject>(SceneObject::CreateSphere(name, s, glm::vec3(), glm::vec3(), glm::vec3()));
						scene_obj->SetTransforms(transform);
						scene_obj->SetMotionBlur(m_b); 
						if (tex_map_ind_1 > -1)
							scene_obj->SetTextureMap(texturemaps[tex_map_ind_1]);
						if (tex_map_ind_2 > -1)
							scene_obj->SetTextureMap(texturemaps[tex_map_ind_2]);
						//CH_TRACE(glm::to_string(scene_obj->GetMaterial()->diffuse));
						scene->AddSceneObject(scene_obj->GetName(), scene_obj);
					}
					else if (std::string(child_node->Value()).compare(MESH_INS) == 0)
					{
						std::string name = "Instance_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
						std::string base_mesh_index = "scene_object_" + std::string(child_node->ToElement()->FindAttribute("baseMeshId")->Value());
						bool reset_transform = std::string(child_node->ToElement()->FindAttribute("resetTransform")->Value()).compare("true") == 0;
						/*auto it = scene->m_scene_objects.begin();
						std::advance(it, base_mesh_index);*/
						auto scene_obj = std::shared_ptr<SceneObject>(SceneObject::CreateInstance(name, 
							scene->m_scene_objects[base_mesh_index], reset_transform));

						glm::mat4 transform = glm::mat4(1.0f);
						glm::vec3 m_b = { 0,0,0 };

						int tex_map_ind_1 = -1, tex_map_ind_2 = -1;

						tinyxml2::XMLNode* object_prop = child_node->FirstChild();
						while (object_prop)
						{
							if (std::string(object_prop->Value()).compare(MAT) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								int ind;
								sscanf(data.c_str(), "%d", &ind);
								ind = ind - 1;
								scene_obj->SetMaterial(materials[ind]);
							}
							else if (std::string(object_prop->Value()).compare(TEX) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%d %d", &tex_map_ind_1,  &tex_map_ind_2);
								tex_map_ind_1--; tex_map_ind_2--;;
							}
							else if (std::string(object_prop->Value()).compare(TRANSFORMS) == 0)
							{
								transform = CalculateTransforms(translations, rotations, scalings, composites, object_prop);
							}
							else if (std::string(object_prop->Value()).compare(MOTION_B) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f %f %f", &m_b.x, &m_b.y, &m_b.z);
							}
							object_prop = object_prop->NextSibling();
						}
						if (!reset_transform)
							transform = transform * scene->m_scene_objects[base_mesh_index]->GetModelMatrix();
						scene_obj->SetTransforms(transform);
						scene_obj->SetMotionBlur(m_b);
						if (tex_map_ind_1 > -1)
							scene_obj->SetTextureMap(texturemaps[tex_map_ind_1]);
						if (tex_map_ind_2 > -1)
							scene_obj->SetTextureMap(texturemaps[tex_map_ind_2]);
						scene->AddSceneObject(scene_obj->GetName(), scene_obj);
					}
					else if (std::string(child_node->Value()).compare(L_SPHR) == 0)
					{
						std::string name = "light_sphere_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
						glm::mat4 transform = glm::mat4(1.0f);
						auto s = std::make_shared<LightSphere>(glm::vec3(0,0,0), nullptr, true);

						glm::vec3 m_b = { 0,0,0 };
						glm::vec3 center = { 0,0,0 };
						glm::vec3 rad_scale = { 1,1,1 };

						int tex_map_ind_1 = -1, tex_map_ind_2 = -1;

						tinyxml2::XMLNode* object_prop = child_node->FirstChild();
						while (object_prop)
						{
							if (std::string(object_prop->Value()).compare(MAT) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								int ind;
								sscanf(data.c_str(), "%d", &ind);
								ind = ind - 1;
								s->m_material = materials[ind];
							}
							else if (std::string(object_prop->Value()).compare(TEX) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%d %d", &tex_map_ind_1, &tex_map_ind_2);
								tex_map_ind_1--;
								tex_map_ind_2--;
							}
							else if (std::string(object_prop->Value()).compare("Radiance") == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								glm::vec3 rad;
								sscanf(data.c_str(), "%f %f %f", &rad.x, &rad.y, &rad.z);
								s->m_inten = rad;
							}
							else if (std::string(object_prop->Value()).compare("Center") == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								int ind;
								sscanf(data.c_str(), "%d", &ind);
								ind = ind - 1;
								center = *vertices[ind];
							}
							else if (std::string(object_prop->Value()).compare(RAD) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								float r;
								sscanf(data.c_str(), "%f", &r);
								rad_scale = r * rad_scale;
							}
							else if (std::string(object_prop->Value()).compare(TRANSFORMS) == 0)
							{
								transform = CalculateTransforms(translations, rotations, scalings, composites, object_prop);
							}
							else if (std::string(object_prop->Value()).compare(MOTION_B) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f %f %f", &m_b.x, &m_b.y, &m_b.z);
							}
							object_prop = object_prop->NextSibling();
						}
						transform *= glm::scale(glm::translate(glm::mat4(1.0f), center), rad_scale);
						auto scene_obj = std::shared_ptr<SceneObject>(SceneObject::CreateSphere(name, s, glm::vec3(), glm::vec3(), glm::vec3()));
						scene_obj->SetTransforms(transform);
						scene_obj->SetMotionBlur(m_b);
						if (tex_map_ind_1 > -1)
							scene_obj->SetTextureMap(texturemaps[tex_map_ind_1]);
						if (tex_map_ind_2 > -1)
							scene_obj->SetTextureMap(texturemaps[tex_map_ind_2]);
						scene->AddSceneObject(scene_obj->GetName(), scene_obj);
						scene->AddLight(name, s);
					}
					else if (std::string(child_node->Value()).compare(L_MESH) == 0)
					{
						tinyxml2::XMLNode* object_prop = child_node->FirstChild();
						std::string name = "light_mesh_" + std::string(child_node->ToElement()->FindAttribute("id")->Value());
						std::shared_ptr<Mesh> mesh = nullptr;
						int mat_ind = 0, tex_map_ind_1 = -1, tex_map_ind_2 = -1;
						auto shading_mode = child_node->ToElement()->FindAttribute(SHADING_M.c_str());
						bool smooth_normals = (shading_mode ? (std::string(shading_mode->Value()).compare(SMOOTH) == 0 ?
							true : false) : false);
						std::string().compare(SMOOTH) == 0;
						glm::mat4 transform = glm::mat4(1.0f);

						glm::vec3 m_b = { 0,0,0 };
						glm::vec3 rad = { 0,0,0 };

						bool has_tex = false;
						std::vector<unsigned int> tex_inds(0);

						while (object_prop)
						{
							if (std::string(object_prop->Value()).compare(MAT) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%d", &mat_ind);
								mat_ind--;
							}
							else if (std::string(object_prop->Value()).compare("Radiance") == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f %f %f", &rad.x, &rad.y, &rad.z);
							}
							else if (std::string(object_prop->Value()).compare(TEX) == 0)
							{
								has_tex = true;
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%d %d", &tex_map_ind_1, &tex_map_ind_2);
								tex_map_ind_1--; tex_map_ind_2--;;
							}
							else if (std::string(object_prop->Value()).compare(FACES) == 0)
							{
								auto ply_file_path = object_prop->ToElement()->FindAttribute("plyFile");
								if (ply_file_path)
								{
									mesh = std::shared_ptr<Mesh>(LoadMeshFromPly(file_path.substr(0, found + 1) + std::string(ply_file_path->Value())));
									//mesh = new Mesh(mesh_verts, mesh_normals, mesh_uvs, std::vector<glm::vec3>(), mesh_indices);
								}
								else
								{
									std::string data = object_prop->FirstChild()->Value();
									std::vector<std::shared_ptr<glm::vec3>> mesh_verts;
									std::vector<std::shared_ptr<glm::vec2>> mesh_uvs;
									std::vector<std::shared_ptr<glm::vec3>> mesh_normals;
									std::vector<unsigned int> mesh_indices;

									int t_off = 0, v_off = 0;

									auto vertex_offset = object_prop->ToElement()->FindAttribute("vertexOffset");
									auto texture_offset = object_prop->ToElement()->FindAttribute("textureOffset");

									if (vertex_offset != NULL)
										v_off = vertex_offset->IntValue();

									if (texture_offset != NULL)
										t_off = texture_offset->IntValue();

									std::string line;
									std::istringstream stream(data);
									mesh_verts = vertices;
									if (has_tex)
									{
										mesh_uvs = texture_coords;
									}
									mesh_normals.reserve(vertices.size());
									mesh_normals.resize(vertices.size());


									while (std::getline(stream, line)) //read faces line by line
									{
										unsigned int ind[3];
										std::istringstream iss(line);
										iss >> ind[0] >> ind[1] >> ind[2];

										if (iss) {

											glm::vec3 a = (*vertices[ind[0] - 1 + v_off] - *vertices[ind[1] - 1 + v_off]);
											glm::vec3 b = (*vertices[ind[0] - 1 + v_off] - *vertices[ind[2] - 1 + v_off]);

											glm::vec3 normal = glm::vec3(0.0f, 0.0f, 0.0f);//glm::normalize(glm::cross(a, b));//calculate face normal

											normal = (glm::cross(-a, -b));

											for (int j = 0; j < 3; j++)
											{
												mesh_indices.push_back(ind[j] - 1 + v_off);
												mesh_normals[ind[j] - 1 + v_off] = std::make_shared<glm::vec3>(glm::normalize(normal));
												if (has_tex)
												{
													if (v_off != 0)
													{
														tex_inds.push_back(ind[j] - 1 + t_off);
													}
												}
											}
										}
									}

									mesh = std::shared_ptr<Mesh>
										(new Mesh(mesh_verts, mesh_normals, mesh_uvs, std::vector<std::shared_ptr<glm::vec3>>(), mesh_indices));
								}

							}
							else if (std::string(object_prop->Value()).compare(TRANSFORMS) == 0)
							{
								transform = CalculateTransforms(translations, rotations, scalings, composites, object_prop);
							}
							else if (std::string(object_prop->Value()).compare(MOTION_B) == 0)
							{
								std::string data = object_prop->FirstChild()->Value();
								sscanf(data.c_str(), "%f %f %f", &m_b.x, &m_b.y, &m_b.z);
							}
							object_prop = object_prop->NextSibling();
						}
						auto scene_obj = std::shared_ptr<SceneObject>
							(new SceneObject(mesh, name, glm::vec3(), glm::vec3(), glm::vec3(1.0, 1.0, 1.0), SHAPE_T::triangle, tex_inds, rad));
						scene_obj->SetMaterial(materials[mat_ind]);
						scene_obj->SetTransforms(transform);
						scene_obj->SetMotionBlur(m_b);
						if (tex_map_ind_1 > -1)
							scene_obj->SetTextureMap(texturemaps[tex_map_ind_1]);
						if (tex_map_ind_2 > -1)
							scene_obj->SetTextureMap(texturemaps[tex_map_ind_2]);

						if (smooth_normals)
						{
							scene_obj->SmoothNormals();
						}
						std::vector<std::shared_ptr<LightTriangle>> tmp;
						int i = 0;
						for (auto tri : scene_obj->m_mesh->m_shapes)
						{
							tmp.push_back(std::dynamic_pointer_cast<LightTriangle>(tri));
						}
						//Add as LightMesh
						auto li_mesh = std::make_shared<LightMesh>(rad, tmp);
						scene->AddLight(name, li_mesh);
					}
					child_node = child_node->NextSibling();
				}
			}
			node = node->NextSibling();
		}
		return scene;
	}
}