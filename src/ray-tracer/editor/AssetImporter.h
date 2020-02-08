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
        static Mesh* LoadMeshFromOBJ(const std::string& file_name);

        Texture * LoadTexture(const std::string & file_name);

		static Scene* LoadSceneFromXML(Shader* shader, const std::string& file_name);

    private:
    };
}