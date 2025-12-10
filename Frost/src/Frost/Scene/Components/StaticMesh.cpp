#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Asset/AssetManager.h"
#include "Frost/Asset/ModelFactory.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Core/Application.h"

namespace Frost::Component
{
    StaticMesh::StaticMesh()
    {
        _config = MeshSourceCube{ 1.0f };
        _Generate();
    }

    StaticMesh::StaticMesh(const MeshConfig& newConfig) : _config{ newConfig }
    {
        _Generate();
    }

    StaticMesh::StaticMesh(const MeshConfig& newConfig, bool forceNewModel) :
        _config{ newConfig }, _forceNewModel(forceNewModel)
    {
        _Generate();
    }

    void StaticMesh::SetMeshConfig(const MeshConfig& newConfig)
    {
        _config = newConfig;
        _Generate();
    }

    void StaticMesh::Reload()
    {
        _Generate();
    }

    void StaticMesh::_Generate()
    {
        _model.reset();

        if (std::holds_alternative<MeshSourceFile>(_config))
        {
            auto& params = std::get<MeshSourceFile>(_config);

            if (params.filepath.empty())
            {
                return;
            }

            std::filesystem::path loadPath = params.filepath;
            bool found = false;

            if (std::filesystem::exists(loadPath))
            {
                found = true;
            }
            else
            {
                std::filesystem::path projectPath = Application::GetProjectDirectory() / params.filepath;
                if (std::filesystem::exists(projectPath))
                {
                    loadPath = projectPath;
                    found = true;
                }
            }

            if (found)
            {
                try
                {
                    if (_forceNewModel)
                    {
                        _model = std::make_shared<Model>();
                        _model->SetStatus(AssetStatus::Loading);
                        _model->LoadCPU(loadPath.string());
                        _model->UploadGPU();
                    }
                    else
                    {
                        _model = AssetManager::LoadAsset<Model>(loadPath.string());
                    }
                }
                catch (const std::exception& e)
                {
                    FT_ENGINE_ERROR("StaticMesh: Failed to load '{0}': {1}", loadPath.string(), e.what());
                }
            }
            else
            {
                FT_ENGINE_WARN("StaticMesh: File not found: {0}. (Project Dir: {1})",
                               params.filepath.string(),
                               Application::GetProjectDirectory().string());
            }
        }
        else if (auto* p = std::get_if<MeshSourceCube>(&_config))
        {
            _model = ModelFactory::CreateCube(*p);
        }
        else if (std::holds_alternative<MeshSourceSphere>(_config))
        {
            auto& params = std::get<MeshSourceSphere>(_config);
            _model = ModelFactory::CreateSphere(params.radius, params.slices, params.rings);
        }
        else if (std::holds_alternative<MeshSourcePlane>(_config))
        {
            auto& params = std::get<MeshSourcePlane>(_config);
            _model = ModelFactory::CreatePlane(params.width, params.depth);
        }
        else if (std::holds_alternative<MeshSourceCylinder>(_config))
        {
            auto& params = std::get<MeshSourceCylinder>(_config);
            _model = ModelFactory::CreateCylinder(
                params.bottomRadius, params.topRadius, params.height, params.slices, params.stacks);
        }
        else if (auto* p = std::get_if<MeshSourceHeightMap>(&_config))
        {
            auto& params = std::get<MeshSourceHeightMap>(_config);

            if (params.texturePath.empty())
            {
                return;
            }

            std::filesystem::path loadPath = params.texturePath;
            bool found = false;

            if (std::filesystem::exists(loadPath))
            {
                found = true;
            }
            else
            {
                std::filesystem::path projectPath = Application::GetProjectDirectory() / params.texturePath;
                if (std::filesystem::exists(projectPath))
                {
                    loadPath = projectPath;
                    found = true;
                }
            }

            if (found)
            {
                try
                {
                    MeshSourceHeightMap config = *p;
                    config.texturePath = loadPath;

                    _model = ModelFactory::CreateFromHeightMap(config);
                }
                catch (const std::exception& e)
                {
                    FT_ENGINE_ERROR("StaticMesh: Failed to load '{0}': {1}", loadPath.string(), e.what());
                }
            }
            else
            {
                FT_ENGINE_WARN("StaticMesh: File not found: {0}. (Project Dir: {1})",
                               params.texturePath.string(),
                               Application::GetProjectDirectory().string());
            }
        }
    }
} // namespace Frost::Component
