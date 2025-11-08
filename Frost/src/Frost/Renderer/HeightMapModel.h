#pragma once

#include "Frost/Renderer/Model.h"
#include "Frost/Renderer/Texture.h"
#include "Frost/Renderer/DX11/TextureDX11.h"
#include "Frost/Renderer/Material.h"
#include <Frost.h>


namespace Frost
{
    class HeightMapModel : public Model
    {
    public:
        HeightMapModel(
            const std::string& heightmapPath,
            Material material,
            TextureChannel channel = TextureChannel::R,
            int chunkSize = 10,
            float heightScale = 1.0f
        );


    private:
        int width = 0;
        int height = 0;

        void GenerateFromHeightmap(
            TextureDX11* heightTexture,
            const Material& material,
            TextureChannel channel,
            int chunkSize,
            float heightScale
        );

        void GenerateMeshFromHeightmap(
            int xMin,
            int xMax,
            int yMin,
            int yMax,
            TextureDX11* heightTexture,
            const Material& material,
            TextureChannel channel,
            float heightScale
        );
    };
} // namespace Frost
