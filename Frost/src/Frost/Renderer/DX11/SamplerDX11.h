#pragma once

#include "Frost/Renderer/Sampler.h"

#include <d3d11.h>
#include <wrl/client.h>

namespace Frost
{
	class SamplerDX11 : public Sampler
	{
	public:
		SamplerDX11(const SamplerConfig& config);
		virtual ~SamplerDX11() override = default;

		virtual const SamplerConfig& GetConfig() const override;

		ID3D11SamplerState* GetSamplerState() const;

	private:
		SamplerConfig _config;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerState;
	};
}