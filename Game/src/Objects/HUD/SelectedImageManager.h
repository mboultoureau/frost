#pragma once

#include "Frost.h"
#include <vector>
#include <string>

namespace Frost
{
    class SelectedImageManager
    {
    public:
        SelectedImageManager();
        ~SelectedImageManager() = default;

        void ChangeImageRight();

        void ChangeImageLeft();

        size_t GetSelectedIndex() const { return _currentIndex; }

    private:
        GameObject _hudLeft;
        GameObject _hudMiddle;
        GameObject _hudRight;

        size_t _currentIndex = 0;

        const std::vector<std::string> _imagePaths = { "resources/textures/bike.png",
                                                       "resources/textures/boat.png",
                                                       "resources/textures/plane.png" };

        void _InitializeHUD();
        void _UpdateHUDDisplay();
    };
} // namespace Frost