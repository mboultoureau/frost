/*
#pragma once

#include "Frost/Asset/AssetManager.h"
#include "Frost/Renderer/Material.h"
#include "Frost/Scene/ECS/Component.h"

#include <memory>
#include <string>

namespace Frost::Component
{
    class Billboard : public Component
    {
    public:

        Billboard(float width, float height, std::string pathTexture, Material::FilterMode textureFilter) :
            textureFilepath{ pathTexture }, textureFilter{ textureFilter }, _width{ width }, _height{ height }
        {
            SetTexturePath(pathTexture);
        }


        float GetWidth() const { return _width; }
        float GetHeight() const { return _height; }
        Frost::Math::Vector2 GetSize() const { return { _width, _height }; }

        void SetWidth(float w) { _width = w; }
        void SetHeight(float h) { _height = h; }


        void SetTexturePath(const std::string& newPath)
        {
            textureFilepath = newPath;

            TextureConfig config;
            config.path = newPath;
            config.textureType = TextureType::BILLBOARD;
            texture = AssetManager::LoadAsset(textureFilepath, config);
        }


        std::string textureFilepath;
        std::shared_ptr<Texture> texture;
        Material::FilterMode textureFilter = Material::FilterMode::LINEAR;

        bool centeredY = true;

    private:
        float _width = 1.0f;
        float _height = 1.0f;
    };
} // namespace Frost::Component*/