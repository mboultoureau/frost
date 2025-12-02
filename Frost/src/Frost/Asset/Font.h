#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Renderer/Material.h"
#include "Frost/Asset/Asset.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <../vendor/stb/stb_truetype.h>

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

    class Font : public Asset
    {
    public:
        Font(const std::string& path);
        ~Font();

        const CharacterMetric& GetCharacterMetric(char c) const;
        std::shared_ptr<Texture> GetAtlasTexture() const { return _atlasTexture; }

        Material::FilterMode GetFilterMode() const { return Material::FilterMode::LINEAR; }

    private:
        std::shared_ptr<Texture> _atlasTexture;
        std::unordered_map<char, CharacterMetric> _metrics;
        void _LoadFont(const std::string& path);
    };
} // namespace Frost