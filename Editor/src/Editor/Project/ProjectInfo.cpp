#include "Editor/Project/ProjectInfo.h"
#include "Frost.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>

using namespace Frost;

namespace fs = std::filesystem;

namespace YAML
{
    // Helper to serialize/deserialize std::vector<bool>
    template<>
    struct convert<std::vector<bool>>
    {
        static Node encode(const std::vector<bool>& v)
        {
            Node node;
            for (const auto& b : v)
            {
                node.push_back(b);
            }
            return node;
        }

        static bool decode(const Node& node, std::vector<bool>& v)
        {
            if (!node.IsSequence())
            {
                return false;
            }
            v.clear();
            for (const auto& element : node)
            {
                v.push_back(element.as<bool>());
            }
            return true;
        }
    };
} // namespace YAML

namespace Editor
{
    void ProjectInfo::Clear()
    {
        _config = ProjectConfig{};
        _projectDir.clear();
        _projectFilePath.clear();
    }

    bool ProjectInfo::LoadFromPath(const std::string& path)
    {
        fs::path fsPath(path);

        if (fs::is_directory(fsPath))
        {
            for (const auto& entry : fs::directory_iterator(fsPath))
            {
                if (entry.path().extension() == ".frost")
                {
                    _projectFilePath = entry.path().string();
                    break;
                }
            }
        }
        else if (fsPath.extension() == ".frost")
        {
            _projectFilePath = path;
        }

        if (_projectFilePath.empty() || !fs::exists(_projectFilePath))
        {
            return false;
        }

        _projectDir = fs::path(_projectFilePath).parent_path().string();

        // Parse YAML
        try
        {
            YAML::Node data = YAML::LoadFile(_projectFilePath);
            auto projectNode = data["project"];
            if (projectNode)
            {
                // General
                _config.name = projectNode["name"].as<std::string>();
                _config.version = projectNode["version"].as<std::string>();
                _config.startScene = projectNode["start_scene"].as<std::string>();
                _config.assetDirectory = projectNode["asset_directory"].as<std::string>();
                _config.sourceDirectory = projectNode["source_directory"].as<std::string>();
                _config.scriptingModule = projectNode["scripting_module"].as<std::string>();

                // Physics
                auto physicsNode = projectNode["physics"];
                if (physicsNode)
                {
                    if (physicsNode["broadphase_layers"])
                    {
                        for (const auto& node : physicsNode["broadphase_layers"])
                        {
                            BroadPhaseLayerSetting layer;
                            layer.name = node["name"].as<std::string>();
                            layer.layerId = node["id"].as<uint8_t>();
                            _config.broadPhaseLayers.push_back(layer);
                        }
                    }

                    if (physicsNode["object_layers"])
                    {
                        for (const auto& node : physicsNode["object_layers"])
                        {
                            ObjectLayerSetting layer;
                            layer.name = node["name"].as<std::string>();
                            layer.layerId = node["id"].as<JPH::ObjectLayer>();
                            layer.broadPhaseLayerId = node["broadphase_id"].as<uint8_t>();
                            _config.objectLayers.push_back(layer);
                        }
                    }

                    _config.broadPhaseCollisionMatrix = physicsNode["broadphase_matrix"].as<std::vector<bool>>();
                    _config.objectCollisionMatrix = physicsNode["object_matrix"].as<std::vector<bool>>();
                    _config.objectBroadPhaseCollisionMatrix =
                        physicsNode["object_broadphase_matrix"].as<std::vector<bool>>();
                }

                return true;
            }
        }
        catch (YAML::ParserException& e)
        {
            std::cerr << "Failed to load project file: " << e.what() << std::endl;
        }

        return false;
    }

    bool ProjectInfo::Save()
    {
        try
        {
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "project" << YAML::Value << YAML::BeginMap;
            {
                // General
                out << YAML::Key << "name" << YAML::Value << _config.name;
                out << YAML::Key << "version" << YAML::Value << _config.version;
                out << YAML::Key << "start_scene" << YAML::Value << _config.startScene;
                out << YAML::Key << "asset_directory" << YAML::Value << _config.assetDirectory;
                out << YAML::Key << "source_directory" << YAML::Value << _config.sourceDirectory;
                out << YAML::Key << "scripting_module" << YAML::Value << _config.scriptingModule;

                // Physics
                out << YAML::Key << "physics" << YAML::Value << YAML::BeginMap;
                {
                    // BroadPhase Layers
                    out << YAML::Key << "broadphase_layers" << YAML::Value << YAML::BeginSeq;
                    for (const auto& layer : _config.broadPhaseLayers)
                    {
                        out << YAML::BeginMap;
                        out << YAML::Key << "name" << YAML::Value << layer.name;
                        out << YAML::Key << "id" << YAML::Value << (int)layer.layerId;
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;

                    // Object Layers
                    out << YAML::Key << "object_layers" << YAML::Value << YAML::BeginSeq;
                    for (const auto& layer : _config.objectLayers)
                    {
                        out << YAML::BeginMap;
                        out << YAML::Key << "name" << YAML::Value << layer.name;
                        out << YAML::Key << "id" << YAML::Value << layer.layerId;
                        out << YAML::Key << "broadphase_id" << YAML::Value << (int)layer.broadPhaseLayerId;
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;

                    // Matrices
                    out << YAML::Key << "broadphase_matrix" << YAML::Value << _config.broadPhaseCollisionMatrix;
                    out << YAML::Key << "object_matrix" << YAML::Value << _config.objectCollisionMatrix;
                    out << YAML::Key << "object_broadphase_matrix" << YAML::Value
                        << _config.objectBroadPhaseCollisionMatrix;
                }
                out << YAML::EndMap;
            }
            out << YAML::EndMap;
            out << YAML::EndMap;

            std::ofstream fout(_projectFilePath);
            fout << out.c_str();

            return true;
        }
        catch (std::exception& e)
        {
            std::cerr << "Error saving project: " << e.what() << std::endl;
            return false;
        }
    }

    bool ProjectInfo::CreateNewProject(const std::string& name,
                                       const std::string& parentDir,
                                       const std::string& templatePath)
    {
        fs::path targetDir = fs::path(parentDir) / name;
        fs::path templateDir(templatePath);

        if (fs::exists(targetDir))
        {
            FT_ENGINE_CRITICAL("Project directory '{}' already exists.", targetDir.string());
            return false;
        }

        if (!fs::exists(templateDir))
        {
            FT_ENGINE_CRITICAL("Template directory '{}' does not exist.", templateDir.string());
            return false;
        }

        try
        {
            fs::copy(templateDir, targetDir, fs::copy_options::recursive);

            fs::path projectFile = targetDir / "project.frost";

            if (fs::exists(targetDir / "template.frost"))
            {
                fs::rename(targetDir / "template.frost", projectFile);
            }

            ProjectConfig config;

            config.name = name;
            config.version = VERSION;
            config.startScene = "DefaultScene";
            config.assetDirectory = "assets";
            config.sourceDirectory = "src";
            config.scriptingModule = "bin/Debug-Windows-x64/" + name + "Logic.dll";

            // BroadPhase Layers: NON_MOVING, MOVING, DEBRIS, SENSOR, UNUSED
            config.broadPhaseLayers = {
                { "NON_MOVING", 0 }, { "MOVING", 1 }, { "DEBRIS", 2 }, { "SENSOR", 3 }, { "UNUSED", 4 }
            };

            // Object Layers: NON_MOVING, MOVING, DEBRIS, SENSOR
            config.objectLayers = {
                { "NON_MOVING", 0, 0 }, { "MOVING", 1, 1 }, { "DEBRIS", 2, 2 }, { "SENSOR", 3, 3 }
            };

            const int bpCount = config.broadPhaseLayers.size();
            const int objCount = config.objectLayers.size();

            config.broadPhaseCollisionMatrix.resize(bpCount * bpCount, false);
            config.objectCollisionMatrix.resize(objCount * objCount, false);
            config.objectBroadPhaseCollisionMatrix.resize(objCount * bpCount, false);

            auto setBPCollision = [&](int l1, int l2)
            {
                config.broadPhaseCollisionMatrix[l1 * bpCount + l2] = true;
                config.broadPhaseCollisionMatrix[l2 * bpCount + l1] = true;
            };
            setBPCollision(0, 1); // NON_MOVING <-> MOVING
            setBPCollision(0, 2); // NON_MOVING <-> DEBRIS
            setBPCollision(1, 1); // MOVING <-> MOVING
            setBPCollision(1, 2); // MOVING <-> DEBRIS

            auto setObjCollision = [&](int l1, int l2)
            {
                config.objectCollisionMatrix[l1 * objCount + l2] = true;
                config.objectCollisionMatrix[l2 * objCount + l1] = true;
            };
            setObjCollision(0, 1); // NON_MOVING <-> MOVING
            setObjCollision(0, 2); // NON_MOVING <-> DEBRIS
            setObjCollision(1, 1); // MOVING <-> MOVING
            setObjCollision(1, 2); // MOVING <-> DEBRIS

            for (int i = 0; i < objCount; ++i)
            {
                for (int j = 0; j < bpCount; ++j)
                {
                    if (config.broadPhaseCollisionMatrix[config.objectLayers[i].broadPhaseLayerId * bpCount + j])
                    {
                        config.objectBroadPhaseCollisionMatrix[i * bpCount + j] = true;
                    }
                }
            }

            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "project" << YAML::Value << YAML::BeginMap;
            {
                out << YAML::Key << "name" << YAML::Value << config.name;
                out << YAML::Key << "version" << YAML::Value << config.version;
                out << YAML::Key << "start_scene" << YAML::Value << config.startScene;
                out << YAML::Key << "asset_directory" << YAML::Value << config.assetDirectory;
                out << YAML::Key << "source_directory" << YAML::Value << config.sourceDirectory;

                // Serialize Physics Settings
                out << YAML::Key << "physics" << YAML::Value << YAML::BeginMap;
                {
                    out << YAML::Key << "broadphase_layers" << YAML::Value << YAML::BeginSeq;
                    for (const auto& layer : config.broadPhaseLayers)
                    {
                        out << YAML::BeginMap;
                        out << YAML::Key << "name" << YAML::Value << layer.name;
                        out << YAML::Key << "id" << YAML::Value << (int)layer.layerId;
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;

                    out << YAML::Key << "object_layers" << YAML::Value << YAML::BeginSeq;
                    for (const auto& layer : config.objectLayers)
                    {
                        out << YAML::BeginMap;
                        out << YAML::Key << "name" << YAML::Value << layer.name;
                        out << YAML::Key << "id" << YAML::Value << layer.layerId;
                        out << YAML::Key << "broadphase_id" << YAML::Value << (int)layer.broadPhaseLayerId;
                        out << YAML::EndMap;
                    }
                    out << YAML::EndSeq;

                    out << YAML::Key << "broadphase_matrix" << YAML::Value << config.broadPhaseCollisionMatrix;
                    out << YAML::Key << "object_matrix" << YAML::Value << config.objectCollisionMatrix;
                    out << YAML::Key << "object_broadphase_matrix" << YAML::Value
                        << config.objectBroadPhaseCollisionMatrix;
                }
                out << YAML::EndMap;
            }
            out << YAML::EndMap;
            out << YAML::EndMap;

            std::ofstream fout(projectFile);
            fout << out.c_str();

            return true;
        }
        catch (std::exception& e)
        {
            std::cerr << "Error creating project: " << e.what() << std::endl;
            return false;
        }
    }
} // namespace Editor