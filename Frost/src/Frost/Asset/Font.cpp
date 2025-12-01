#define STB_TRUETYPE_IMPLEMENTATION
#include "Frost/Asset/Font.h"
#include "Frost/Asset/Texture.h"
#include "Frost/Renderer/Format.h"
#include "Frost/Debugging/Logger.h"
#include <fstream>
#include <vector>

#ifdef FT_PLATFORM_WINDOWS
#include "Frost/Renderer/DX11/TextureDX11.h"
#endif

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
    Font::Font(const std::string& path)
    {
        _LoadFont(path);
    }

    Font::~Font() {}

    void Font::_LoadFont(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            FT_ENGINE_ERROR("Failed to open font file: {}", path);
            return;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> font_data(size);
        if (!file.read((char*)font_data.data(), size))
        {
            FT_ENGINE_ERROR("Failed to read font file content: {}", path);
            return;
        }

        std::vector<uint8_t> pixels(ATLAS_SIZE * ATLAS_SIZE);

        stbtt_bakedchar char_data[CHAR_COUNT];

        int bakeResult = stbtt_BakeFontBitmap(font_data.data(),
                                              0,
                                              FONT_SIZE_PX,
                                              pixels.data(),
                                              ATLAS_SIZE,
                                              ATLAS_SIZE,
                                              FIRST_CHAR,
                                              CHAR_COUNT,
                                              char_data);

        if (bakeResult <= 0)
        {
            FT_ENGINE_ERROR("stbtt_BakeFontBitmap failed for font: {}", path);
            return;
        }

#ifdef FT_PLATFORM_WINDOWS

        _atlasTexture = std::make_shared<TextureDX11>(ATLAS_SIZE, ATLAS_SIZE, Format::R8_UNORM, pixels.data(), path);
#else
#error "Font texture creation not implemented for this platform"
#endif

        if (!_atlasTexture)
        {
            FT_ENGINE_ERROR("TextureDX11 failed to initialize for font: {}", path);
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

    const CharacterMetric& Font::GetCharacterMetric(char c) const
    {
        if (_metrics.count(c))
        {
            return _metrics.at(c);
        }
        return ::DEFAULT_METRIC;
    }
} // namespace Frost