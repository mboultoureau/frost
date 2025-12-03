#pragma once

namespace Editor
{
    class EditorPanel
    {
    public:
        virtual void Draw(float deltaTime) = 0;
        bool IsOpen() const { return _isOpen; }

    protected:
        bool _isOpen = true;
    };
} // namespace Editor