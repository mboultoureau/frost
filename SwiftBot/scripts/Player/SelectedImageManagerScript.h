#pragma once

#include "Frost.h"
#include <vector>
#include <string>
namespace GameLogic
{
    class SelectedImageManagerScript : public Frost::Scripting::Script
    {
    public:
        SelectedImageManagerScript(const SelectedImageManagerScript&) = delete;
        SelectedImageManagerScript& operator=(const SelectedImageManagerScript&) = delete;

        explicit SelectedImageManagerScript() = default;

        void OnCreate() override;
        void OnUpdate(float deltaTime) override {}

        void ChangeImageRight();

        void ChangeImageLeft();

        size_t GetSelectedIndex() const { return _currentIndex; }

    private:
        Frost::GameObject _hudLeft;
        Frost::GameObject _hudMiddle;
        Frost::GameObject _hudRight;

        size_t _currentIndex = 0;

        const std::vector<std::string> _imagePaths = { "assets/Player/bike.png",
                                                       "assets/Player/boat.png",
                                                       "assets/Player/plane.png" };

        void _InitializeHUD();
        void _UpdateHUDDisplay();
    };
} // namespace GameLogic