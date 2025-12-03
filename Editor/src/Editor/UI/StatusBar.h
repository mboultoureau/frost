#pragma once

#include "Editor/UI/EditorPanel.h"

namespace Editor
{
    class StatusBar : public EditorPanel
    {
    public:
        void Draw(float deltaTime) override;
    };
} // namespace Editor
