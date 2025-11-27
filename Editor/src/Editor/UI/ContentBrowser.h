#pragma once

#include "Editor/UI/EditorPanel.h"

namespace Editor
{
    class ContentBrowser : public EditorPanel
    {
    public:
        ContentBrowser() = default;
        ~ContentBrowser() = default;

        void Draw() override;
    };
} // namespace Editor
