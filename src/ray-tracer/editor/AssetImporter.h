#pragma once

#include <ray-tracer/main/Scene.h>
#include "Settings.h"

#include <thirdparty/tinyxml2/tinyxml2.h>

namespace CHR
{

    class AssetImporter
    {
    public:
        static Mesh* LoadMeshFromOBJ(const std::string& file_name, 
			glm::vec3 t = { 0,0,0 },
			glm::vec3 r = { 0,0,0 },
			glm::vec3 s = { 1,1,1 });

        static Texture* LoadTexture(const std::string & file_name);

		static Mesh* LoadMeshFromPly(std::string ply_path);

		static Scene* LoadSceneFromXML(Shader* shader, const std::string& file_name);

    private:
    };
}