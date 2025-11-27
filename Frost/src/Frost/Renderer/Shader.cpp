#include "Frost/Renderer/Shader.h"
#include "Frost/Renderer/DX11/ShaderDX11.h"

namespace Frost
{
    std::shared_ptr<Shader> Shader::Create(const ShaderDesc& desc)
    {
#ifdef FT_PLATFORM_WINDOWS
        return std::make_shared<ShaderDX11>(desc);
#else
        FT_ENGINE_ASSERT(false, "Platform not supported for Shaders yet!");
        return nullptr;
#endif
    }
} // namespace Frost
