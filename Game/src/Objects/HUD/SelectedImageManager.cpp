#include "SelectedImageManager.h"
#include "Frost/Scene/Components/HUDImage.h"
#include "Frost/Renderer/Material.h"
#include "Game.h"

using namespace Frost;
using namespace Frost::Component;

SelectedImageManager::SelectedImageManager()
{
    _InitializeHUD();

    _currentIndex = 0;
    _UpdateHUDDisplay();
}

void
SelectedImageManager::_InitializeHUD()
{
    Scene& _scene = Game::GetScene();

    _hudLeft = _scene.CreateGameObject("HUD_Left_Icon");
    _hudMiddle = _scene.CreateGameObject("HUD_Middle_Icon");
    _hudRight = _scene.CreateGameObject("HUD_Right_Icon");

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

    _hudLeft.AddComponent<HUDImage>(viewportLeft, INIT_PATH, Material::FilterMode::POINT);
    _hudMiddle.AddComponent<HUDImage>(viewportMiddle, INIT_PATH, Material::FilterMode::POINT);
    _hudRight.AddComponent<HUDImage>(viewportRight, INIT_PATH, Material::FilterMode::POINT);
}

void
SelectedImageManager::ChangeImageRight()
{
    if (_imagePaths.empty())
        return;

    _currentIndex = (_currentIndex + 1) % _imagePaths.size();

    _UpdateHUDDisplay();
}

void
SelectedImageManager::ChangeImageLeft()
{
    if (_imagePaths.empty())
        return;

    size_t numImages = _imagePaths.size();
    _currentIndex = (_currentIndex - 1 + numImages) % numImages;

    _UpdateHUDDisplay();
}

void
SelectedImageManager::_UpdateHUDDisplay()
{

    size_t numImages = _imagePaths.size();

    size_t current = _currentIndex;
    size_t prev = (current - 1 + numImages) % numImages;
    size_t next = (current + 1) % numImages;

    std::string pathLeft = _imagePaths[prev];
    std::string pathMiddle = _imagePaths[current];
    std::string pathRight = _imagePaths[next];

    _hudLeft.GetComponent<HUDImage>().SetTexturePath(pathLeft);

    _hudMiddle.GetComponent<HUDImage>().SetTexturePath(pathMiddle);

    _hudRight.GetComponent<HUDImage>().SetTexturePath(pathRight);
}