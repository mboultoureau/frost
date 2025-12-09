#include "Editor/Utils/PhysicsCodeGenerator.h"

#include <fstream>
#include <algorithm>
#include <sstream>
#include <filesystem>
#include <iostream>

namespace Editor
{
    bool PhysicsCodeGenerator::Generate(const ProjectConfig& config, const std::string& outputDirectory)
    {
        std::filesystem::path dir(outputDirectory);
        if (!std::filesystem::exists(dir))
        {
            std::filesystem::create_directories(dir);
        }

        std::filesystem::path headerPath = dir / "GeneratedPhysicsLayers.h";
        std::filesystem::path sourcePath = dir / "GeneratedPhysicsLayers.cpp";

        // Header (.h)
        std::ofstream headerFile(headerPath);
        if (!headerFile.is_open())
            return false;
        headerFile << _GenerateHeaderContent(config);
        headerFile.close();

        // Source (.cpp)
        std::ofstream sourceFile(sourcePath);
        if (!sourceFile.is_open())
            return false;
        sourceFile << _GenerateSourceContent(config);
        sourceFile.close();

        return true;
    }

    std::string PhysicsCodeGenerator::_SanitizeName(const std::string& name)
    {
        std::string result = name;
        // Replace with underscores and uppercase
        std::transform(result.begin(),
                       result.end(),
                       result.begin(),
                       [](unsigned char c) -> char
                       {
                           if (std::isalnum(c))
                               return static_cast<char>(std::toupper(c));
                           return '_';
                       });

        // If the name starts with a digit, prepend an underscore
        if (!result.empty() && std::isdigit(result[0]))
        {
            result = "_" + result;
        }
        return result;
    }

    std::string PhysicsCodeGenerator::_GenerateHeaderContent(const ProjectConfig& config)
    {
        std::stringstream ss;

        ss << "#pragma once\n\n";
        ss << "#include <Jolt/Jolt.h>\n";
        ss << "#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>\n";
        ss << "#include <Jolt/Physics/Collision/ObjectLayer.h>\n\n";

        // Object Layers Namespace
        ss << "namespace ObjectLayers\n{\n";
        for (const auto& layer : config.objectLayers)
        {
            ss << "\tstatic constexpr JPH::ObjectLayer " << _SanitizeName(layer.name) << " = " << layer.layerId
               << ";\n";
        }
        ss << "\tstatic constexpr JPH::uint NUM_LAYERS = " << config.objectLayers.size() << ";\n";
        ss << "}\n\n";

        // BroadPhase Layers Namespace
        ss << "namespace BroadPhaseLayers\n{\n";
        for (const auto& layer : config.broadPhaseLayers)
        {
            ss << "\tstatic constexpr JPH::BroadPhaseLayer " << _SanitizeName(layer.name) << "(" << (int)layer.layerId
               << ");\n";
        }
        ss << "\tstatic constexpr JPH::uint NUM_LAYERS = " << config.broadPhaseLayers.size() << ";\n";
        ss << "}\n\n";

        // Classes Definitions
        ss << "// Class mapping Object Layers to BroadPhase Layers\n";
        ss << "class GameBroadPhaseLayerInterface final : public JPH::BroadPhaseLayerInterface\n{\n";
        ss << "public:\n";
        ss << "\tGameBroadPhaseLayerInterface();\n";
        ss << "\tvirtual JPH::uint GetNumBroadPhaseLayers() const override;\n";
        ss << "\tvirtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override;\n";
        ss << "#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)\n";
        ss << "\tvirtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;\n";
        ss << "#endif\n";
        ss << "private:\n";
        ss << "\tJPH::BroadPhaseLayer mObjectToBroadPhase[ObjectLayers::NUM_LAYERS];\n";
        ss << "};\n\n";

        ss << "// Class determining if two Object Layers collide\n";
        ss << "class GameObjectLayerPairFilter : public JPH::ObjectLayerPairFilter\n{\n";
        ss << "public:\n";
        ss << "\tvirtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override;\n";
        ss << "};\n\n";

        ss << "// Class determining if an Object Layer collides with a BroadPhase Layer\n";
        ss << "class GameObjectVsBroadPhaseLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter\n{\n";
        ss << "public:\n";
        ss << "\tvirtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const "
              "override;\n";
        ss << "};\n";

        return ss.str();
    }

    std::string PhysicsCodeGenerator::_GenerateSourceContent(const ProjectConfig& config)
    {
        std::stringstream ss;

        ss << "#include \"GeneratedPhysicsLayers.h\"\n";
        ss << "#include <cassert>\n\n";

        // BroadPhaseLayerInterface Implementation
        ss << "GameBroadPhaseLayerInterface::GameBroadPhaseLayerInterface()\n{\n";
        for (const auto& objLayer : config.objectLayers)
        {
            std::string bpName = "UNKNOWN";
            for (const auto& bpLayer : config.broadPhaseLayers)
            {
                if (bpLayer.layerId == objLayer.broadPhaseLayerId)
                {
                    bpName = _SanitizeName(bpLayer.name);
                    break;
                }
            }
            ss << "\tmObjectToBroadPhase[ObjectLayers::" << _SanitizeName(objLayer.name)
               << "] = BroadPhaseLayers::" << bpName << ";\n";
        }
        ss << "}\n\n";

        ss << "JPH::uint GameBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const\n{\n";
        ss << "\treturn BroadPhaseLayers::NUM_LAYERS;\n}\n\n";

        ss << "JPH::BroadPhaseLayer GameBroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) "
              "const\n{\n";
        ss << "\t// JPH_ASSERT(inLayer < ObjectLayers::NUM_LAYERS);\n";
        ss << "\treturn mObjectToBroadPhase[inLayer];\n}\n\n";

        ss << "#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)\n";
        ss << "const char* GameBroadPhaseLayerInterface::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) "
              "const\n{\n";
        ss << "\tswitch ((JPH::BroadPhaseLayer::Type)inLayer)\n\t{\n";
        for (const auto& bpLayer : config.broadPhaseLayers)
        {
            ss << "\t\tcase (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::" << _SanitizeName(bpLayer.name)
               << ": return \"" << bpLayer.name << "\";\n";
        }
        ss << "\t\tdefault: return \"Unknown\";\n";
        ss << "\t}\n}\n";
        ss << "#endif\n\n";

        // ObjectLayerPairFilter Implementation
        ss << "bool GameObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) "
              "const\n{\n";
        ss << "\tswitch (inObject1)\n\t{\n";

        int numObjLayers = (int)config.objectLayers.size();
        for (int i = 0; i < numObjLayers; ++i)
        {
            ss << "\t\tcase ObjectLayers::" << _SanitizeName(config.objectLayers[i].name) << ":\n";
            ss << "\t\t\treturn ";

            bool first = true;
            bool collidesWithAny = false;

            for (int j = 0; j < numObjLayers; ++j)
            {
                int matrixIdx = i * numObjLayers + j;
                if (matrixIdx < config.objectCollisionMatrix.size() && config.objectCollisionMatrix[matrixIdx])
                {
                    if (!first)
                        ss << " || ";
                    ss << "inObject2 == ObjectLayers::" << _SanitizeName(config.objectLayers[j].name);
                    first = false;
                    collidesWithAny = true;
                }
            }

            if (!collidesWithAny)
                ss << "false";
            ss << ";\n";
        }

        ss << "\t\tdefault:\n\t\t\treturn false;\n";
        ss << "\t}\n}\n\n";

        // ObjectVsBroadPhaseLayerFilter Implementation
        ss << "bool GameObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer "
              "inLayer2) const\n{\n";
        ss << "\tswitch (inLayer1)\n\t{\n";

        int numBPLayers = (int)config.broadPhaseLayers.size();
        for (int i = 0; i < numObjLayers; ++i)
        {
            ss << "\t\tcase ObjectLayers::" << _SanitizeName(config.objectLayers[i].name) << ":\n";
            ss << "\t\t\treturn ";

            bool first = true;
            bool collidesWithAny = false;

            for (int j = 0; j < numBPLayers; ++j)
            {
                int matrixIdx = i * numBPLayers + j;
                if (matrixIdx < config.objectBroadPhaseCollisionMatrix.size() &&
                    config.objectBroadPhaseCollisionMatrix[matrixIdx])
                {
                    if (!first)
                        ss << " || ";
                    ss << "inLayer2 == BroadPhaseLayers::" << _SanitizeName(config.broadPhaseLayers[j].name);
                    first = false;
                    collidesWithAny = true;
                }
            }

            if (!collidesWithAny)
                ss << "false";
            ss << ";\n";
        }

        ss << "\t\tdefault:\n\t\t\treturn false;\n";
        ss << "\t}\n}\n";

        return ss.str();
    }
} // namespace Editor
