#pragma once

#include "Frost/Asset/AssetManager.h"
#include "Frost/Asset/HeightMapModel.h"
#include "Frost/Asset/Model.h"
#include "Frost/Core/Application.h"
#include "Frost/Scene/ECS/Component.h"

#include <memory>
#include <string>

namespace Frost::Component
{
    struct StaticMesh : public Component
    {
        std::shared_ptr<Model> model;

        StaticMesh(std::shared_ptr<Model> model) : model{ model } {}

        StaticMesh(std::string modelFilepath) : model{ AssetManager::LoadAsset<Model>(modelFilepath, modelFilepath) } {}
    };

} // namespace Frost::Component
