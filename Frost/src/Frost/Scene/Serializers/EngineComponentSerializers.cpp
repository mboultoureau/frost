#include "Frost/Scene/Serializers/EngineComponentSerializer.h"
#include "Frost/Scene/Serializers/SerializationSystem.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Utils/SerializerUtils.h"

using namespace Frost;
using namespace Frost::Component;

namespace Frost
{
    void EngineComponentSerializer::RegisterEngineComponents()
    {
        // Meta
        SerializationSystem::RegisterComponent<Meta>(
            "Meta",
            // Write YAML
            [](YAML::Emitter& out, GameObject go)
            {
                auto& comp = go.GetComponent<Meta>();
                out << YAML::Key << "Name" << YAML::Value << comp.name;
            },
            // Read YAML
            [](const YAML::Node& node, GameObject& go)
            {
                auto& tc = go.GetComponent<Meta>();
                tc.name = node["Name"].as<std::string>();
            },
            // Write Binary
            [](std::ostream& out, GameObject go)
            {
                auto& comp = go.GetComponent<Meta>();
                WriteBinaryString(out, comp.name);
            },
            // Read Binary
            [](std::istream& in, GameObject& go)
            {
                auto& comp = go.GetComponent<Meta>();
                comp.name = ReadBinaryString(in);
            });

        // Transform
        SerializationSystem::RegisterComponent<Component::Transform>(
            "Transform",
            // Write YAML
            [](YAML::Emitter& out, GameObject go)
            {
                auto& tc = go.GetComponent<Component::Transform>();
                out << YAML::Key << "Position" << YAML::Value << tc.position;
                out << YAML::Key << "Rotation" << YAML::Value << tc.rotation;
                out << YAML::Key << "Scale" << YAML::Value << tc.scale;
            },
            // Read YAML
            [](const YAML::Node& node, GameObject& go)
            {
                auto& tc = go.GetComponent<Component::Transform>();
                tc.position = node["Position"].as<decltype(tc.position)>();
                tc.rotation = node["Rotation"].as<decltype(tc.rotation)>();
                tc.scale = node["Scale"].as<decltype(tc.scale)>();
            },
            // Write Binary
            [](std::ostream& out, GameObject go)
            {
                auto& tc = go.GetComponent<Component::Transform>();
                WriteBinary(out, tc.position);
                WriteBinary(out, tc.rotation);
                WriteBinary(out, tc.scale);
            },
            // Read Binary
            [](std::istream& in, GameObject& go)
            {
                auto& tc = go.GetComponent<Component::Transform>();
                ReadBinary(in, tc.position);
                ReadBinary(in, tc.rotation);
                ReadBinary(in, tc.scale);
            });

        // World Transform
        SerializationSystem::RegisterComponent<Component::WorldTransform>(
            "WorldTransform",
            [](YAML::Emitter& out, GameObject go) {},
            [](const YAML::Node& node, GameObject& go) {},
            [](std::ostream& out, GameObject go) {},
            [](std::istream& in, GameObject& go) {});

        // StaticMesh
        SerializationSystem::RegisterComponent<StaticMesh>(
            "StaticMesh",
            // Write YAML
            [](YAML::Emitter& out, GameObject go)
            {
                auto& mesh = go.GetComponent<StaticMesh>();

                out << YAML::Key << "Type" << YAML::Value << (int)mesh.GetMeshConfig().index();
                out << YAML::Key << "Params" << YAML::BeginMap;

                if (auto* p = std::get_if<MeshSourceFile>(&mesh.GetMeshConfig()))
                {
                    out << YAML::Key << "Path" << YAML::Value << p->filepath.generic_string();
                }
                else if (auto* p = std::get_if<MeshSourceCube>(&mesh.GetMeshConfig()))
                {
                    out << YAML::Key << "Size" << YAML::Value << p->size;
                    out << YAML::Key << "Segments" << YAML::Value << p->segments;
                    out << YAML::Key << "Bevel" << YAML::Value << p->bevelRadius;
                }
                else if (auto* p = std::get_if<MeshSourceSphere>(&mesh.GetMeshConfig()))
                {
                    out << YAML::Key << "Radius" << YAML::Value << p->radius;
                    out << YAML::Key << "Rings" << YAML::Value << p->rings;
                    out << YAML::Key << "Slices" << YAML::Value << p->slices;
                }
                else if (auto* p = std::get_if<MeshSourcePlane>(&mesh.GetMeshConfig()))
                {
                    out << YAML::Key << "Width" << YAML::Value << p->width;
                    out << YAML::Key << "Depth" << YAML::Value << p->depth;
                }
                else if (auto* p = std::get_if<MeshSourceCylinder>(&mesh.GetMeshConfig()))
                {
                    out << YAML::Key << "BottomRadius" << YAML::Value << p->bottomRadius;
                    out << YAML::Key << "TopRadius" << YAML::Value << p->topRadius;
                    out << YAML::Key << "Height" << YAML::Value << p->height;
                    out << YAML::Key << "Slices" << YAML::Value << p->slices;
                    out << YAML::Key << "Stacks" << YAML::Value << p->stacks;
                }
                else if (auto* p = std::get_if<MeshSourceHeightMap>(&mesh.GetMeshConfig()))
                {
                    out << YAML::Key << "TexturePath" << YAML::Value << p->texturePath;
                    out << YAML::Key << "Width" << YAML::Value << p->width;
                    out << YAML::Key << "Depth" << YAML::Value << p->depth;
                    out << YAML::Key << "MinHeight" << YAML::Value << p->minHeight;
                    out << YAML::Key << "MaxHeight" << YAML::Value << p->maxHeight;
                    out << YAML::Key << "SegmentWidth" << YAML::Value << p->segmentsWidth;
                    out << YAML::Key << "SegmentDepth" << YAML::Value << p->segmentsDepth;
                }

                out << YAML::EndMap;
            },
            // Read YAML
            [](const YAML::Node& node, GameObject& go)
            {
                auto& mesh = go.GetComponent<StaticMesh>();
                int typeIndex = node["Type"].as<int>();
                auto paramsNode = node["Params"];

                switch ((MeshType)typeIndex)
                {
                    case MeshType::File:
                    {
                        mesh.SetMeshConfig(MeshSourceFile{ paramsNode["Path"].as<std::string>() });
                        break;
                    }
                    case MeshType::Cube:
                    {
                        float size = paramsNode["Size"].as<float>();

                        Math::Vector3 segments = { 1.0f, 1.0f, 1.0f };
                        if (paramsNode["Segments"])
                            segments = paramsNode["Segments"].as<Math::Vector3>();

                        float bevel = 0.0f;
                        if (paramsNode["Bevel"])
                            bevel = paramsNode["Bevel"].as<float>();

                        mesh.SetMeshConfig(MeshSourceCube{ size, segments, bevel });
                        break;
                    }
                    case MeshType::Sphere:
                    {
                        mesh.SetMeshConfig(MeshSourceSphere{ paramsNode["Radius"].as<float>(),
                                                             paramsNode["Rings"].as<uint32_t>(),
                                                             paramsNode["Slices"].as<uint32_t>() });
                        break;
                    }
                    case MeshType::Plane:
                    {
                        mesh.SetMeshConfig(
                            MeshSourcePlane{ paramsNode["Width"].as<float>(), paramsNode["Depth"].as<float>() });
                        break;
                    }
                    case MeshType::Cylinder:
                    {
                        mesh.SetMeshConfig(MeshSourceCylinder{ paramsNode["BottomRadius"].as<float>(),
                                                               paramsNode["TopRadius"].as<float>(),
                                                               paramsNode["Height"].as<float>(),
                                                               paramsNode["Slices"].as<uint32_t>(),
                                                               paramsNode["Stacks"].as<uint32_t>() });
                        break;
                    }
                    case MeshType::HeightMap:
                    {
                        MeshSourceHeightMap hm;
                        hm.texturePath = paramsNode["TexturePath"].as<std::string>();
                        hm.width = paramsNode["Width"].as<float>();
                        hm.depth = paramsNode["Depth"].as<float>();
                        hm.minHeight = paramsNode["MinHeight"].as<float>();
                        hm.maxHeight = paramsNode["MaxHeight"].as<float>();

                        if (paramsNode["SegmentWidth"])
                            hm.segmentsWidth = paramsNode["SegmentWidth"].as<uint32_t>();
                        if (paramsNode["SegmentDepth"])
                            hm.segmentsDepth = paramsNode["SegmentDepth"].as<uint32_t>();

                        mesh.SetMeshConfig(hm);
                        break;
                    }
                }
            },
            // Write Binary
            [](std::ostream& out, GameObject go)
            {
                auto& mesh = go.GetComponent<StaticMesh>();
                int typeIndex = (int)mesh.GetMeshConfig().index();
                out.write((char*)&typeIndex, sizeof(int));

                if (auto* p = std::get_if<MeshSourceFile>(&mesh.GetMeshConfig()))
                {
                    WriteBinaryString(out, p->filepath.generic_string());
                }
                else if (auto* p = std::get_if<MeshSourceCube>(&mesh.GetMeshConfig()))
                {
                    out.write((char*)&p->size, sizeof(float));
                    WriteBinary(out, p->segments);
                    out.write((char*)&p->bevelRadius, sizeof(float));
                }
                else if (auto* p = std::get_if<MeshSourceSphere>(&mesh.GetMeshConfig()))
                {
                    out.write((char*)&p->radius, sizeof(float));
                    out.write((char*)&p->rings, sizeof(uint32_t));
                    out.write((char*)&p->slices, sizeof(uint32_t));
                }
                else if (auto* p = std::get_if<MeshSourcePlane>(&mesh.GetMeshConfig()))
                {
                    out.write((char*)&p->width, sizeof(float));
                    out.write((char*)&p->depth, sizeof(float));
                }
                else if (auto* p = std::get_if<MeshSourceCylinder>(&mesh.GetMeshConfig()))
                {
                    out.write((char*)&p->bottomRadius, sizeof(float));
                    out.write((char*)&p->topRadius, sizeof(float));
                    out.write((char*)&p->height, sizeof(float));
                    out.write((char*)&p->slices, sizeof(uint32_t));
                    out.write((char*)&p->stacks, sizeof(uint32_t));
                }
                else if (auto* p = std::get_if<MeshSourceHeightMap>(&mesh.GetMeshConfig()))
                {
                    WriteBinaryString(out, p->texturePath);
                    out.write((char*)&p->width, sizeof(float));
                    out.write((char*)&p->depth, sizeof(float));
                    out.write((char*)&p->minHeight, sizeof(float));
                    out.write((char*)&p->maxHeight, sizeof(float));
                    out.write((char*)&p->segmentsWidth, sizeof(uint32_t));
                    out.write((char*)&p->segmentsDepth, sizeof(uint32_t));
                }
            },
            // Read Binary
            [](std::istream& in, GameObject& go)
            {
                auto& mesh = go.GetComponent<StaticMesh>();
                int typeIndex;
                in.read((char*)&typeIndex, sizeof(int));

                switch ((MeshType)typeIndex)
                {
                    case MeshType::File:
                    {
                        std::string path = ReadBinaryString(in);
                        mesh.SetMeshConfig(MeshSourceFile{ path });
                        break;
                    }
                    case MeshType::Cube:
                    {
                        float size, bevel;
                        Math::Vector3 segments;

                        in.read((char*)&size, sizeof(float));
                        ReadBinary(in, segments);
                        in.read((char*)&bevel, sizeof(float));

                        mesh.SetMeshConfig(MeshSourceCube{ size, segments, bevel });
                        break;
                    }
                    case MeshType::Sphere:
                    {
                        float radius;
                        uint32_t rings, slices;
                        in.read((char*)&radius, sizeof(float));
                        in.read((char*)&rings, sizeof(uint32_t));
                        in.read((char*)&slices, sizeof(uint32_t));
                        mesh.SetMeshConfig(MeshSourceSphere{ radius, rings, slices });
                        break;
                    }
                    case MeshType::Plane:
                    {
                        float width, depth;
                        in.read((char*)&width, sizeof(float));
                        in.read((char*)&depth, sizeof(float));
                        mesh.SetMeshConfig(MeshSourcePlane{ width, depth });
                        break;
                    }
                    case MeshType::Cylinder:
                    {
                        float bottomR, topR, h;
                        uint32_t sl, st;
                        in.read((char*)&bottomR, sizeof(float));
                        in.read((char*)&topR, sizeof(float));
                        in.read((char*)&h, sizeof(float));
                        in.read((char*)&sl, sizeof(uint32_t));
                        in.read((char*)&st, sizeof(uint32_t));
                        mesh.SetMeshConfig(MeshSourceCylinder{ bottomR, topR, h, sl, st });
                        break;
                    }
                    case MeshType::HeightMap:
                    {
                        MeshSourceHeightMap hm;
                        hm.texturePath = ReadBinaryString(in);
                        in.read((char*)&hm.width, sizeof(float));
                        in.read((char*)&hm.depth, sizeof(float));
                        in.read((char*)&hm.minHeight, sizeof(float));
                        in.read((char*)&hm.maxHeight, sizeof(float));
                        in.read((char*)&hm.segmentsWidth, sizeof(uint32_t));
                        in.read((char*)&hm.segmentsDepth, sizeof(uint32_t));
                        mesh.SetMeshConfig(hm);
                        break;
                    }
                }
            });
    }
} // namespace Frost