#include "Frost/Scene/Serializers/EngineComponentSerializer.h"
#include "Frost/Scene/Serializers/SerializationSystem.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Utils/SerializerUtils.h"

using namespace Frost;
using namespace Frost::Component;

namespace Frost
{
    void EngineComponentSerializer::RegisterEngineComponents()
    {
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

        SerializationSystem::RegisterComponent<Component::WorldTransform>(
            "WorldTransform",
            [](YAML::Emitter& out, GameObject go) {},
            [](const YAML::Node& node, GameObject& go) {},
            [](std::ostream& out, GameObject go) {},
            [](std::istream& in, GameObject& go) {});
    }
} // namespace Frost