#pragma once

#include "Frost/Asset/Model.h"
#include "Frost/Scene/ECS/Component.h"
#include "Frost/Asset/MeshConfig.h"
#include "Frost/Scene/Components/VirtualCamera.h"

#include <memory>
#include <string>
#include <variant>
#include <filesystem>

namespace Frost::Component
{
    using MeshConfig = std::variant<MeshSourceFile,
                                    MeshSourceCube,
                                    MeshSourceSphere,
                                    MeshSourcePlane,
                                    MeshSourceCylinder,
                                    MeshSourceHeightMap>;

    enum class MeshType : int
    {
        File = 0,
        Cube = 1,
        Sphere = 2,
        Plane = 3,
        Cylinder = 4,
        HeightMap = 5
    };

    class StaticMesh : public Component
    {
    public:
        StaticMesh();
        StaticMesh(const MeshConfig& newConfig);
        StaticMesh(const MeshConfig& newConfig, bool forceNewModel);

        std::shared_ptr<Model>& GetModel() { return _model; }
        const std::shared_ptr<Model>& GetModel() const { return _model; }

        MeshConfig& GetMeshConfig() { return _config; }
        const MeshConfig& GetMeshConfig() const { return _config; }
        void SetMeshConfig(const MeshConfig& newConfig);

        MeshType GetType() const { return static_cast<MeshType>(_config.index()); }

        void Reload();

        std::vector<VirtualCamera*> hiddenFromCameras;

    private:
        void _Generate();

    private:
        MeshConfig _config;
        std::shared_ptr<Model> _model;
        bool _forceNewModel = false;
    };

} // namespace Frost::Component
