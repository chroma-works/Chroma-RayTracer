#pragma once

#include <ray-tracer/main/Scene.h>
#include <ray-tracer/main/SceneObject.h>
#include <ray-tracer/editor/Texture.h>

#include <thirdparty/tinyxml2/tinyxml2.h>

namespace Chroma
{

    class AssetImporter
    {
    public:
        static Mesh* LoadMeshFromOBJ(const std::string& file_name, 
			glm::vec3 t = { 0,0,0 },
			glm::vec3 r = { 0,0,0 },
			glm::vec3 s = { 1,1,1 });

        Texture * LoadTexture(const std::string & file_name);

		static Scene* LoadSceneFromXML(Shader* shader, const std::string& file_name);

    private:
    };
}