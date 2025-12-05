#include "Frost/Scene/Serializers/EngineComponentSerializer.h"
#include "Frost/Scene/Serializers/SerializationSystem.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Systems/PhysicSystem.h"
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
                    out << YAML::Key << "TexturePath" << YAML::Value << p->texturePath.generic_string();
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
                    WriteBinaryString(out, p->texturePath.generic_string());
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

        SerializationSystem::RegisterComponent<Light>(
            "Light",
            // Write YAML
            [](YAML::Emitter& out, GameObject go)
            {
                auto& light = go.GetComponent<Light>();

                out << YAML::Key << "Type" << YAML::Value << (int)light.config.index();
                out << YAML::Key << "Color" << YAML::Value << light.color;
                out << YAML::Key << "Intensity" << YAML::Value << light.intensity;

                out << YAML::Key << "Params" << YAML::BeginMap;

                if (auto* d = std::get_if<LightDirectional>(&light.config))
                {
                    out << YAML::Key << "CastShadows" << YAML::Value << d->castShadows;
                    out << YAML::Key << "ShadowBias" << YAML::Value << d->shadowBias;
                }
                else if (auto* p = std::get_if<LightPoint>(&light.config))
                {
                    out << YAML::Key << "Radius" << YAML::Value << p->radius;
                    out << YAML::Key << "Falloff" << YAML::Value << p->falloff;
                }
                else if (auto* s = std::get_if<LightSpot>(&light.config))
                {
                    out << YAML::Key << "Range" << YAML::Value << s->range;
                    out << YAML::Key << "InnerAngle" << YAML::Value
                        << Math::Angle<Math::Degree>(s->innerConeAngle).value();
                    out << YAML::Key << "OuterAngle" << YAML::Value
                        << Math::Angle<Math::Degree>(s->outerConeAngle).value();
                }

                out << YAML::EndMap;
            },
            // Read YAML
            [](const YAML::Node& node, GameObject& go)
            {
                auto& light = go.GetComponent<Light>();

                int typeIndex = node["Type"].as<int>();
                light.color = node["Color"].as<Math::Color3>();
                light.intensity = node["Intensity"].as<float>();

                auto params = node["Params"];

                switch ((LightType)typeIndex)
                {
                    case LightType::Directional:
                    {
                        LightDirectional d;
                        if (params["CastShadows"])
                            d.castShadows = params["CastShadows"].as<bool>();
                        if (params["ShadowBias"])
                            d.shadowBias = params["ShadowBias"].as<float>();
                        light.config = d;
                        break;
                    }
                    case LightType::Point:
                    {
                        LightPoint p;
                        if (params["Radius"])
                            p.radius = params["Radius"].as<float>();
                        if (params["Falloff"])
                            p.falloff = params["Falloff"].as<float>();
                        light.config = p;
                        break;
                    }
                    case LightType::Spot:
                    {
                        LightSpot s;
                        if (params["Range"])
                            s.range = params["Range"].as<float>();

                        if (params["InnerAngle"])
                            s.innerConeAngle =
                                Math::Angle<Math::Radian>(Math::Angle<Math::Degree>(params["InnerAngle"].as<float>()));

                        if (params["OuterAngle"])
                            s.outerConeAngle =
                                Math::Angle<Math::Radian>(Math::Angle<Math::Degree>(params["OuterAngle"].as<float>()));

                        light.config = s;
                        break;
                    }
                }
            },
            // Write Binary
            [](std::ostream& out, GameObject go)
            {
                auto& light = go.GetComponent<Light>();
                int typeIndex = (int)light.config.index();

                out.write((char*)&typeIndex, sizeof(int));
                WriteBinary(out, light.color);
                out.write((char*)&light.intensity, sizeof(float));

                if (auto* d = std::get_if<LightDirectional>(&light.config))
                {
                    out.write((char*)&d->castShadows, sizeof(bool));
                    out.write((char*)&d->shadowBias, sizeof(float));
                }
                else if (auto* p = std::get_if<LightPoint>(&light.config))
                {
                    out.write((char*)&p->radius, sizeof(float));
                    out.write((char*)&p->falloff, sizeof(float));
                }
                else if (auto* s = std::get_if<LightSpot>(&light.config))
                {
                    out.write((char*)&s->range, sizeof(float));
                    float inner = s->innerConeAngle.value();
                    float outer = s->outerConeAngle.value();
                    out.write((char*)&inner, sizeof(float));
                    out.write((char*)&outer, sizeof(float));
                }
            },
            // Read Binary
            [](std::istream& in, GameObject& go)
            {
                auto& light = go.GetComponent<Light>();
                int typeIndex;

                in.read((char*)&typeIndex, sizeof(int));
                ReadBinary(in, light.color);
                in.read((char*)&light.intensity, sizeof(float));

                switch ((LightType)typeIndex)
                {
                    case LightType::Directional:
                    {
                        LightDirectional d;
                        in.read((char*)&d.castShadows, sizeof(bool));
                        in.read((char*)&d.shadowBias, sizeof(float));
                        light.config = d;
                        break;
                    }
                    case LightType::Point:
                    {
                        LightPoint p;
                        in.read((char*)&p.radius, sizeof(float));
                        in.read((char*)&p.falloff, sizeof(float));
                        light.config = p;
                        break;
                    }
                    case LightType::Spot:
                    {
                        LightSpot s;
                        float inner, outer;
                        in.read((char*)&s.range, sizeof(float));
                        in.read((char*)&inner, sizeof(float));
                        in.read((char*)&outer, sizeof(float));
                        s.innerConeAngle = Math::Angle<Math::Radian>(inner);
                        s.outerConeAngle = Math::Angle<Math::Radian>(outer);
                        light.config = s;
                        break;
                    }
                }
            });

        // RigidBody
        SerializationSystem::RegisterComponent<RigidBody>(
            "RigidBody",
            // Write YAML
            [](YAML::Emitter& out, GameObject go)
            {
                auto& rb = go.GetComponent<RigidBody>();

                out << YAML::Key << "MotionType" << YAML::Value << static_cast<int>(rb.motionType);
                out << YAML::Key << "IsSensor" << YAML::Value << rb.isSensor;
                out << YAML::Key << "AllowSleeping" << YAML::Value << rb.allowSleeping;
                out << YAML::Key << "Friction" << YAML::Value << rb.friction;
                out << YAML::Key << "Restitution" << YAML::Value << rb.restitution;
                out << YAML::Key << "LinearDamping" << YAML::Value << rb.linearDamping;
                out << YAML::Key << "AngularDamping" << YAML::Value << rb.angularDamping;
                out << YAML::Key << "GravityFactor" << YAML::Value << rb.gravityFactor;
                out << YAML::Key << "OverrideMassProperties" << YAML::Value
                    << static_cast<int>(rb.overrideMassProperties);
                out << YAML::Key << "Mass" << YAML::Value << rb.mass;

                out << YAML::Key << "LockPositionX" << YAML::Value << rb.lockPositionX;
                out << YAML::Key << "LockPositionY" << YAML::Value << rb.lockPositionY;
                out << YAML::Key << "LockPositionZ" << YAML::Value << rb.lockPositionZ;
                out << YAML::Key << "LockRotationX" << YAML::Value << rb.lockRotationX;
                out << YAML::Key << "LockRotationY" << YAML::Value << rb.lockRotationY;
                out << YAML::Key << "LockRotationZ" << YAML::Value << rb.lockRotationZ;

                out << YAML::Key << "ShapeType" << YAML::Value << static_cast<int>(rb.shape.index());
                out << YAML::Key << "ShapeParams" << YAML::BeginMap;
                std::visit(
                    [&out](auto&& arg)
                    {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, ShapeBox>)
                        {
                            out << YAML::Key << "HalfExtent" << YAML::Value << arg.halfExtent;
                            out << YAML::Key << "ConvexRadius" << YAML::Value << arg.convexRadius;
                        }
                        else if constexpr (std::is_same_v<T, ShapeSphere>)
                        {
                            out << YAML::Key << "Radius" << YAML::Value << arg.radius;
                        }
                        else if constexpr (std::is_same_v<T, ShapeCapsule>)
                        {
                            out << YAML::Key << "HalfHeight" << YAML::Value << arg.halfHeight;
                            out << YAML::Key << "Radius" << YAML::Value << arg.radius;
                        }
                        else if constexpr (std::is_same_v<T, ShapeCylinder>)
                        {
                            out << YAML::Key << "HalfHeight" << YAML::Value << arg.halfHeight;
                            out << YAML::Key << "Radius" << YAML::Value << arg.radius;
                            out << YAML::Key << "ConvexRadius" << YAML::Value << arg.convexRadius;
                        }
                    },
                    rb.shape);
                out << YAML::EndMap;
            },
            // Read YAML
            [](const YAML::Node& node, GameObject& go)
            {
                auto& rb = go.GetComponent<RigidBody>();

                rb.motionType = static_cast<RigidBody::MotionType>(node["MotionType"].as<int>());
                rb.isSensor = node["IsSensor"].as<bool>();
                rb.allowSleeping = node["AllowSleeping"].as<bool>();
                rb.friction = node["Friction"].as<float>();
                rb.restitution = node["Restitution"].as<float>();
                rb.linearDamping = node["LinearDamping"].as<float>();
                rb.angularDamping = node["AngularDamping"].as<float>();
                rb.gravityFactor = node["GravityFactor"].as<float>();
                rb.overrideMassProperties =
                    static_cast<RigidBody::OverrideMassProperties>(node["OverrideMassProperties"].as<int>());
                rb.mass = node["Mass"].as<float>();

                rb.lockPositionX = node["LockPositionX"].as<bool>();
                rb.lockPositionY = node["LockPositionY"].as<bool>();
                rb.lockPositionZ = node["LockPositionZ"].as<bool>();
                rb.lockRotationX = node["LockRotationX"].as<bool>();
                rb.lockRotationY = node["LockRotationY"].as<bool>();
                rb.lockRotationZ = node["LockRotationZ"].as<bool>();

                int shapeType = node["ShapeType"].as<int>();
                auto params = node["ShapeParams"];
                switch (static_cast<CollisionShapeType>(shapeType))
                {
                    case CollisionShapeType::Box:
                    {
                        ShapeBox s;
                        s.halfExtent = params["HalfExtent"].as<Math::Vector3>();
                        s.convexRadius = params["ConvexRadius"].as<float>();
                        rb.shape = s;
                        break;
                    }
                    case CollisionShapeType::Sphere:
                    {
                        ShapeSphere s;
                        s.radius = params["Radius"].as<float>();
                        rb.shape = s;
                        break;
                    }
                    case CollisionShapeType::Capsule:
                    {
                        ShapeCapsule s;
                        s.halfHeight = params["HalfHeight"].as<float>();
                        s.radius = params["Radius"].as<float>();
                        rb.shape = s;
                        break;
                    }
                    case CollisionShapeType::Cylinder:
                    {
                        ShapeCylinder s;
                        s.halfHeight = params["HalfHeight"].as<float>();
                        s.radius = params["Radius"].as<float>();
                        s.convexRadius = params["ConvexRadius"].as<float>();
                        rb.shape = s;
                        break;
                    }
                }

                if (auto* physicSystem = go.GetScene()->GetSystem<PhysicSystem>())
                {
                    physicSystem->NotifyRigidBodyUpdate(*go.GetScene(), go);
                }
            },
            // Write Binary
            [](std::ostream& out, GameObject go)
            {
                auto& rb = go.GetComponent<RigidBody>();

                int motionType = static_cast<int>(rb.motionType);
                out.write(reinterpret_cast<const char*>(&motionType), sizeof(int));

                out.write(reinterpret_cast<const char*>(&rb.isSensor), sizeof(bool));
                out.write(reinterpret_cast<const char*>(&rb.allowSleeping), sizeof(bool));
                out.write(reinterpret_cast<const char*>(&rb.friction), sizeof(float));
                out.write(reinterpret_cast<const char*>(&rb.restitution), sizeof(float));
                out.write(reinterpret_cast<const char*>(&rb.linearDamping), sizeof(float));
                out.write(reinterpret_cast<const char*>(&rb.angularDamping), sizeof(float));
                out.write(reinterpret_cast<const char*>(&rb.gravityFactor), sizeof(float));

                int overrideMass = static_cast<int>(rb.overrideMassProperties);
                out.write(reinterpret_cast<const char*>(&overrideMass), sizeof(int));
                out.write(reinterpret_cast<const char*>(&rb.mass), sizeof(float));

                out.write(reinterpret_cast<const char*>(&rb.lockPositionX), sizeof(bool));
                out.write(reinterpret_cast<const char*>(&rb.lockPositionY), sizeof(bool));
                out.write(reinterpret_cast<const char*>(&rb.lockPositionZ), sizeof(bool));
                out.write(reinterpret_cast<const char*>(&rb.lockRotationX), sizeof(bool));
                out.write(reinterpret_cast<const char*>(&rb.lockRotationY), sizeof(bool));
                out.write(reinterpret_cast<const char*>(&rb.lockRotationZ), sizeof(bool));

                int shapeType = static_cast<int>(rb.shape.index());
                out.write(reinterpret_cast<const char*>(&shapeType), sizeof(int));

                std::visit(
                    [&out](auto&& arg)
                    {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, ShapeBox>)
                        {
                            WriteBinary(out, arg.halfExtent);
                            out.write(reinterpret_cast<const char*>(&arg.convexRadius), sizeof(float));
                        }
                        else if constexpr (std::is_same_v<T, ShapeSphere>)
                        {
                            out.write(reinterpret_cast<const char*>(&arg.radius), sizeof(float));
                        }
                        else if constexpr (std::is_same_v<T, ShapeCapsule>)
                        {
                            out.write(reinterpret_cast<const char*>(&arg.halfHeight), sizeof(float));
                            out.write(reinterpret_cast<const char*>(&arg.radius), sizeof(float));
                        }
                        else if constexpr (std::is_same_v<T, ShapeCylinder>)
                        {
                            out.write(reinterpret_cast<const char*>(&arg.halfHeight), sizeof(float));
                            out.write(reinterpret_cast<const char*>(&arg.radius), sizeof(float));
                            out.write(reinterpret_cast<const char*>(&arg.convexRadius), sizeof(float));
                        }
                    },
                    rb.shape);
            },
            // Read Binary
            [](std::istream& in, GameObject& go)
            {
                auto& rb = go.GetComponent<RigidBody>();

                int motionType;
                in.read(reinterpret_cast<char*>(&motionType), sizeof(int));
                rb.motionType = static_cast<RigidBody::MotionType>(motionType);

                in.read(reinterpret_cast<char*>(&rb.isSensor), sizeof(bool));
                in.read(reinterpret_cast<char*>(&rb.allowSleeping), sizeof(bool));
                in.read(reinterpret_cast<char*>(&rb.friction), sizeof(float));
                in.read(reinterpret_cast<char*>(&rb.restitution), sizeof(float));
                in.read(reinterpret_cast<char*>(&rb.linearDamping), sizeof(float));
                in.read(reinterpret_cast<char*>(&rb.angularDamping), sizeof(float));
                in.read(reinterpret_cast<char*>(&rb.gravityFactor), sizeof(float));

                int overrideMass;
                in.read(reinterpret_cast<char*>(&overrideMass), sizeof(int));
                rb.overrideMassProperties = static_cast<RigidBody::OverrideMassProperties>(overrideMass);
                in.read(reinterpret_cast<char*>(&rb.mass), sizeof(float));

                in.read(reinterpret_cast<char*>(&rb.lockPositionX), sizeof(bool));
                in.read(reinterpret_cast<char*>(&rb.lockPositionY), sizeof(bool));
                in.read(reinterpret_cast<char*>(&rb.lockPositionZ), sizeof(bool));
                in.read(reinterpret_cast<char*>(&rb.lockRotationX), sizeof(bool));
                in.read(reinterpret_cast<char*>(&rb.lockRotationY), sizeof(bool));
                in.read(reinterpret_cast<char*>(&rb.lockRotationZ), sizeof(bool));

                int shapeType;
                in.read(reinterpret_cast<char*>(&shapeType), sizeof(int));

                switch (static_cast<CollisionShapeType>(shapeType))
                {
                    case CollisionShapeType::Box:
                    {
                        ShapeBox s;
                        ReadBinary(in, s.halfExtent);
                        in.read(reinterpret_cast<char*>(&s.convexRadius), sizeof(float));
                        rb.shape = s;
                        break;
                    }
                    case CollisionShapeType::Sphere:
                    {
                        ShapeSphere s;
                        in.read(reinterpret_cast<char*>(&s.radius), sizeof(float));
                        rb.shape = s;
                        break;
                    }
                    case CollisionShapeType::Capsule:
                    {
                        ShapeCapsule s;
                        in.read(reinterpret_cast<char*>(&s.halfHeight), sizeof(float));
                        in.read(reinterpret_cast<char*>(&s.radius), sizeof(float));
                        rb.shape = s;
                        break;
                    }
                    case CollisionShapeType::Cylinder:
                    {
                        ShapeCylinder s;
                        in.read(reinterpret_cast<char*>(&s.halfHeight), sizeof(float));
                        in.read(reinterpret_cast<char*>(&s.radius), sizeof(float));
                        in.read(reinterpret_cast<char*>(&s.convexRadius), sizeof(float));
                        rb.shape = s;
                        break;
                    }
                }

                if (auto* physicSystem = go.GetScene()->GetSystem<PhysicSystem>())
                {
                    physicSystem->NotifyRigidBodyUpdate(*go.GetScene(), go);
                }
            });
    }
} // namespace Frost