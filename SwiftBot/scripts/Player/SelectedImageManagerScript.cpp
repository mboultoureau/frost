#include "SelectedImageManagerScript.h"
#include "Frost/Scene/Components/UIElement.h"
#include "Frost/Renderer/Material.h"

using namespace Frost;
using namespace Frost::Component;

namespace GameLogic
{
    void SelectedImageManagerScript::OnCreate()
    {
        _InitializeHUD();

        _currentIndex = 0;
        _UpdateHUDDisplay();
    }

    void SelectedImageManagerScript::_InitializeHUD()
    {
        auto _scene = GetGameObject().GetScene();

        _hudLeft = _scene->CreateGameObject("HUD_Left_Icon");
        _hudMiddle = _scene->CreateGameObject("HUD_Middle_Icon");
        _hudRight = _scene->CreateGameObject("HUD_Right_Icon");

        Viewport viewportLeft;
        viewportLeft.height = 0.06f;
        viewportLeft.width = 0.05f;
        viewportLeft.x = 0.75f;
        viewportLeft.y = 0.075f;

        Viewport viewportRight;
        viewportRight.height = 0.06f;
        viewportRight.width = 0.05f;
        viewportRight.x = 0.9f;
        viewportRight.y = 0.075f;

        Viewport viewportMiddle;
        viewportMiddle.height = 0.1f;
        viewportMiddle.width = 0.1f;
        viewportMiddle.x = 0.8f;
        viewportMiddle.y = 0.05f;

        const std::string INIT_PATH = _imagePaths[0];

        _hudLeft.AddComponent<UIElement>(
            UIElement{ .viewport = viewportLeft,
                       .content = UIImage{ .textureFilepath = INIT_PATH, .filter = Material::FilterMode::POINT } });
        _hudMiddle.AddComponent<UIElement>(
            UIElement{ .viewport = viewportMiddle,
                       .content = UIImage{ .textureFilepath = INIT_PATH, .filter = Material::FilterMode::POINT } });
        _hudRight.AddComponent<UIElement>(
            UIElement{ .viewport = viewportRight,
                       .content = UIImage{ .textureFilepath = INIT_PATH, .filter = Material::FilterMode::POINT } });
    }

    void SelectedImageManagerScript::ChangeImageRight()
    {
        if (_imagePaths.empty())
            return;

        _currentIndex = (_currentIndex + 1) % _imagePaths.size();

        _UpdateHUDDisplay();
    }

    void SelectedImageManagerScript::ChangeImageLeft()
    {
        if (_imagePaths.empty())
            return;

        size_t numImages = _imagePaths.size();
        _currentIndex = (_currentIndex - 1 + numImages) % numImages;

        _UpdateHUDDisplay();
    }

    void SelectedImageManagerScript::_UpdateHUDDisplay()
    {
        size_t numImages = _imagePaths.size();

        size_t current = _currentIndex;
        size_t prev = (current - 1 + numImages) % numImages;
        size_t next = (current + 1) % numImages;

        auto updateImage = [](GameObject& hudObject, const std::string& path)
        {
            auto& uiElement = hudObject.GetComponent<UIElement>();
            auto& uiImage = std::get<UIImage>(uiElement.content);
            uiImage.SetTexturePath(path);
            hudObject.SetActive(true);
        };

        updateImage(_hudMiddle, _imagePaths[current]);
        updateImage(_hudLeft, _imagePaths[prev]);
        updateImage(_hudRight, _imagePaths[next]);
    }
} // namespace GameLogic