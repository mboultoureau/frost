#include "Frost/Asset/Font.h"
#include "Frost/Asset/Texture.h"
#include "Frost/Renderer/Format.h"
#include "Frost/Debugging/Logger.h"

#include <fstream>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace
{
    constexpr int ATLAS_SIZE = 512;
    constexpr float FONT_SIZE_PX = 32.0f;
    constexpr int FIRST_CHAR = 32;
    constexpr int CHAR_COUNT = 96;

    const Frost::CharacterMetric DEFAULT_METRIC = { 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
} // namespace

namespace Frost
{
    Font::Font() {}

    Font::~Font() {}

    void Font::LoadCPU(const std::string& path)
    {
        _filePath = path;

        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            FT_ENGINE_ERROR("Failed to open font file: {}", path);
            SetStatus(AssetStatus::Failed);
            return;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> font_data(size);
        if (!file.read((char*)font_data.data(), size))
        {
            FT_ENGINE_ERROR("Failed to read font file content: {}", path);
            SetStatus(AssetStatus::Failed);
            return;
        }

        _stagingPixels.resize(ATLAS_SIZE * ATLAS_SIZE);

        stbtt_bakedchar char_data[CHAR_COUNT];

        int bakeResult = stbtt_BakeFontBitmap(font_data.data(),
                                              0,
                                              FONT_SIZE_PX,
                                              _stagingPixels.data(),
                                              ATLAS_SIZE,
                                              ATLAS_SIZE,
                                              FIRST_CHAR,
                                              CHAR_COUNT,
                                              char_data);

        if (bakeResult <= 0)
        {
            FT_ENGINE_ERROR("stbtt_BakeFontBitmap failed for font: {}", path);
            SetStatus(AssetStatus::Failed);
            return;
        }

        for (int i = 0; i < CHAR_COUNT; ++i)
        {
            const stbtt_bakedchar& bc = char_data[i];
            CharacterMetric metric = {};

            metric.sizeX = bc.x1 - bc.x0;
            metric.sizeY = bc.y1 - bc.y0;

            metric.offsetX = static_cast<int>(bc.xoff);
            metric.offsetY = static_cast<int>(bc.yoff);

            metric.advance = bc.xadvance;

            metric.uvX = static_cast<float>(bc.x0) / ATLAS_SIZE;
            metric.uvY = static_cast<float>(bc.y0) / ATLAS_SIZE;
            metric.uvW = static_cast<float>(bc.x1 - bc.x0) / ATLAS_SIZE;
            metric.uvH = static_cast<float>(bc.y1 - bc.y0) / ATLAS_SIZE;

            _metrics[static_cast<char>(FIRST_CHAR + i)] = metric;
        }

        _metrics['\0'] = DEFAULT_METRIC;
    }

    void Font::UploadGPU()
    {
        if (GetStatus() == AssetStatus::Failed)
        {
            return;
        }

        if (_stagingPixels.empty())
        {
            FT_ENGINE_ERROR("Attempting to upload font to GPU with no pixel data: {}", _filePath);
            SetStatus(AssetStatus::Failed);
            return;
        }

        TextureConfig texConfig{
            .fileData = _stagingPixels,
            .path = _filePath,
            .format = Format::R8_UNORM,
            .channels = 1,
            .width = ATLAS_SIZE,
            .height = ATLAS_SIZE,
        };

        _atlasTexture = Texture::Create(texConfig);

        if (!_atlasTexture)
        {
            FT_ENGINE_ERROR("Texture creation failed for font: {}", _filePath);
            SetStatus(AssetStatus::Failed);
            return;
        }

        _stagingPixels.clear();
        _stagingPixels.shrink_to_fit();

        SetStatus(AssetStatus::Loaded);
    }

    const CharacterMetric& Font::GetCharacterMetric(char c) const
    {
        if (_metrics.count(c))
        {
            return _metrics.at(c);
        }
        return ::DEFAULT_METRIC;
    }
} // namespace Frost