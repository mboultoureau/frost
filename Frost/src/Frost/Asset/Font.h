#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Renderer/Material.h"
#include "Frost/Asset/Asset.h"

#include <string>
#include <memory>
#include <unordered_map>

namespace Frost
{
    class Texture;

    struct CharacterMetric
    {
        int sizeX, sizeY;
        int offsetX, offsetY;
        float advance;
        float uvX, uvY, uvW, uvH;
    };

    class FROST_API Font : public Asset
    {
    public:
        Font();
        ~Font();

        void LoadCPU(const std::string& path);
        void UploadGPU();

        const CharacterMetric& GetCharacterMetric(char c) const;
        std::shared_ptr<Texture> GetAtlasTexture() const { return _atlasTexture; }

        Material::FilterMode GetFilterMode() const { return Material::FilterMode::LINEAR; }

    private:
        std::shared_ptr<Texture> _atlasTexture;
        std::unordered_map<char, CharacterMetric> _metrics;

        std::vector<uint8_t> _stagingPixels;
        std::string _filePath;
    };
} // namespace Frost