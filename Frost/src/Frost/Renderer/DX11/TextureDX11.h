#pragma once

#include "Frost/Asset/Texture.h"
#include "Frost/Utils/File/MemoryMappedFile.h"

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

namespace Frost
{
    class TextureDX11 : public Texture
    {
    public:
        TextureDX11(TextureConfig& config);
        TextureDX11();

        TextureDX11(ID3D11Texture2D* existingTexture, ID3D11RenderTargetView* rtv);
        TextureDX11(ID3D11Texture2D* existingTexture, ID3D11DepthStencilView* dsv);
        TextureDX11(uint32_t width,
                    uint32_t height,
                    Format format,
                    const void* pixelData,
                    const std::string& debugName);

        virtual ~TextureDX11() override = default;

        // Async API
        virtual void LoadCPU(const std::string& path, const TextureConfig& config) override;
        virtual void UploadGPU() override;

        virtual void Bind(Slot slot) const override;
        virtual void* GetRendererID() const override { return _srv.Get(); }
        virtual const std::vector<uint8_t> GetData() const override;

        // Get resources
        ID3D11Texture2D* GetDX11Texture() const { return _texture.Get(); }
        ID3D11ShaderResourceView* GetSRV() const { return _srv.Get(); }
        ID3D11RenderTargetView* GetRTV() const { return _rtv.Get(); }
        ID3D11DepthStencilView* GetDSV() const { return _dsv.Get(); }

        virtual bool SaveToFile(const std::string& path) const override;

    private:
        void _CreateCubemapCPU();

        Microsoft::WRL::ComPtr<ID3D11Texture2D> _texture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _rtv;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _dsv;

        // Temporary CPU data for async loading
        std::unique_ptr<MemoryMappedFile> _mmFile;
        std::vector<uint8_t> _cpuData;
        std::vector<std::vector<uint8_t>> _cpuCubemapData;

        // Cache
        mutable std::vector<uint8_t> _dataCache;
        mutable bool _dataCached = false;
    };
} // namespace Frost