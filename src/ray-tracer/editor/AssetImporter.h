#pragma once

#include <ray-tracer/main/SceneObject.h>
#include <ray-tracer/editor/Texture.h>


namespace Chroma
{

    class AssetImporter
    {
    public:
        static Mesh* LoadMeshFromOBJ(const std::string& file_name);

        Texture * LoadTexture(const std::string & file_name);

    private:
    };
}