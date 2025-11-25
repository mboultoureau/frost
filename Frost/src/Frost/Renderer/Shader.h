#pragma once

#include <string>
#include <memory>

namespace Frost
{
	enum class ShaderType
	{
		None = 0,
		Compute,
		Domain,
		Geometry,
		Hull,
		Pixel,
		Vertex
	};

	struct ShaderDesc
	{
		ShaderType type = ShaderType::None;
		std::string debugName;
		std::string filePath;
	};

	class Shader
	{
	public:
		Shader(const ShaderDesc& desc) : _desc(desc) {}
		virtual ~Shader() = default;

		// Factory
		static std::shared_ptr<Shader> Create(const ShaderDesc& desc);

		// Public accessors for shader properties
		ShaderType GetType() const { return _desc.type; }
		const std::string& GetName() const { return _desc.debugName; }
		const std::string& GetFilePath() const { return _desc.filePath; }

	protected:
		ShaderDesc _desc;
	};
}