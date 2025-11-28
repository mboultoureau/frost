#include "Frost/Asset/Texture.h"
#ifdef FT_PLATFORM_WINDOWS
#include "Frost/Renderer/DX11/TextureDX11.h"
#endif

namespace Frost
{
    std::shared_ptr<Texture> Texture::Create(TextureConfig& config)
    {
#ifdef FT_PLATFORM_WINDOWS
        return std::make_shared<TextureDX11>(config);
#else
#error "Texture creation not implemented for this platform."
#endif
    }
} // namespace Frost
