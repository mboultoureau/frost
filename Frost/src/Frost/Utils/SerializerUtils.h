#pragma once

#include "Frost/Utils/Math/Vector.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

inline YAML::Emitter&
operator<<(YAML::Emitter& out, const Frost::Math::Vector3& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}

inline YAML::Emitter&
operator<<(YAML::Emitter& out, const Frost::Math::Vector4& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}

namespace YAML
{
    template<>
    struct convert<Frost::Math::Vector3>
    {
        static Node encode(const Frost::Math::Vector3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, Frost::Math::Vector3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template<>
    struct convert<Frost::Math::Vector4>
    {
        static Node encode(const Frost::Math::Vector4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node& node, Frost::Math::Vector4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };
} // namespace YAML

namespace Frost
{
    template<typename T>
    inline void ReadBinary(std::istream& in, T& data)
    {
        in.read(reinterpret_cast<char*>(&data), sizeof(T));
    }

    inline std::string ReadBinaryString(std::istream& in)
    {
        size_t size;
        in.read(reinterpret_cast<char*>(&size), sizeof(size_t));
        std::string str(size, '\0');
        in.read(&str[0], size);
        return str;
    }

    // Write
    template<typename T>
    inline void WriteBinary(std::ostream& out, const T& data)
    {
        out.write(reinterpret_cast<const char*>(&data), sizeof(T));
    }

    inline void WriteBinaryString(std::ostream& out, const std::string& str)
    {
        size_t size = str.size();
        out.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
        out.write(str.c_str(), size);
    }
} // namespace Frost