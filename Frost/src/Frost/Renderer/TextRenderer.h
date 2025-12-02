#pragma once

#include "Frost/Asset/Font.h"
#include <string>
#include <memory>
#include <vector>

namespace Frost
{
    class VertexBuffer;

    struct TextVertex
    {
        float x, y, z;
        float u, v;
    };

    class TextRenderer
    {
    public:
        TextRenderer(std::shared_ptr<Font> font);
        ~TextRenderer() = default;

        void SetText(const std::string& text);
        const std::string& GetText() const { return _text; }

        void BindAndDraw();

        std::shared_ptr<Font> GetFont() const { return _font; }
        std::shared_ptr<VertexBuffer> GetVertexBuffer() const { return _vertexBuffer; }

    private:
        std::shared_ptr<Font> _font;
        std::shared_ptr<VertexBuffer> _vertexBuffer;

        std::string _text;
        bool _isDirty = true;

        void _RegenerateMesh();
    };
} // namespace Frost