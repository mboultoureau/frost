#include "Frost/Renderer/PostEffect/ColorCorrectionEffect.h"
#include "Frost/Renderer/DX11/CommandListDX11.h"
#include "Frost/Renderer/DX11/SamplerDX11.h"
#include "Frost/Renderer/DX11/ShaderDX11.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/DX11/TextureDX11.h"

#include <imgui.h>

namespace Frost
{
    struct alignas(16) ColorCorrectionConstants
    {
        float Strength;
        float padding[3];
    };

    ColorCorrectionEffect::ColorCorrectionEffect()
    {
#ifdef FT_PLATFORM_WINDOWS
        ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                              .debugName = "VS_ColorCorrection",
                              .filePath = "../Frost/resources/shaders/PostEffect/VS_ColorCorrection.hlsl" };
        _vertexShader = std::make_unique<ShaderDX11>(vsDesc);

        ShaderDesc psDesc = { .type = ShaderType::Pixel,
                              .debugName = "PS_ColorCorrection",
                              .filePath = "../Frost/resources/shaders/PostEffect/PS_ColorCorrection.hlsl" };
        _pixelShader = std::make_unique<ShaderDX11>(psDesc);
#endif

        auto* renderer = RendererAPI::GetRenderer();
        _constantsBuffer = renderer->CreateBuffer(BufferConfig{ .usage = BufferUsage::CONSTANT_BUFFER,
                                                                .size = sizeof(ColorCorrectionConstants),
                                                                .dynamic = true,
                                                                .debugName = "CB_ColorCorrection" });

        SamplerConfig samplerConfig = { .filter = Filter::MIN_MAG_MIP_LINEAR,
                                        .addressU = AddressMode::CLAMP,
                                        .addressV = AddressMode::CLAMP,
                                        .addressW = AddressMode::CLAMP };
        _sampler = std::make_unique<SamplerDX11>(samplerConfig);

        // Optionnel : Charger une LUT par défaut au démarrage
        LoadLUT("../Frost/resources/textures/lut/neutral.png");
    }

    void ColorCorrectionEffect::LoadLUT(const std::string& path)
    {
        TextureConfig config = {};
        config.path = path;
        config.debugName = "LUT Texture";
        config.loadImmediately = true;
        config.isShaderResource = true;
        config.isRenderTarget = false;

        // Important pour les LUTs : Pas de mipmaps pour éviter le bleeding de couleurs entre les tranches
        config.hasMipmaps = false;

        // Création via votre système d'Asset
        _lutResource = Texture::Create(config);

        // Vérification si le chargement a réussi
        if (_lutResource && _lutResource->GetStatus() == AssetStatus::Loaded)
        {
            _lutTexture = _lutResource.get();
            FT_ENGINE_INFO("ColorCorrection: LUT loaded successfully from {}", path);
        }
        else
        {
            FT_ENGINE_WARN("ColorCorrection: Failed to load LUT from {}", path);
            _lutTexture = nullptr;
        }
    }

    void ColorCorrectionEffect::OnPostRender(float deltaTime,
                                             CommandList* commandList,
                                             Texture* source,
                                             Texture* destination)
    {
        // Si aucune LUT n'est définie, on copie simplement la source vers la destination
        // (Ou on n'applique pas l'effet, selon votre architecture de pipeline)
        if (!_lutTexture)
        {
            // Note : Ici, il faudrait idéalement faire une copie simple (Blit) si cet effet est obligatoire dans la
            // chaine. Pour l'instant, on return, ce qui laissera le buffer précédent (attention aux écrans noirs si
            // c'est le dernier effet).
            return;
        }

        ColorCorrectionConstants constants;
        constants.Strength = _strength;

        _constantsBuffer->UpdateData(commandList, &constants, sizeof(constants));

        commandList->SetRenderTargets(1, &destination, nullptr);
        commandList->SetShader(_vertexShader.get());
        commandList->SetShader(_pixelShader.get());
        commandList->SetInputLayout(nullptr);

        // Slot 0 : Image écran
        commandList->SetTexture(source, 0);

        // Slot 1 : Texture LUT
        if (_lutTexture)
        {
            commandList->SetTexture(_lutTexture, 1);
        }

        commandList->SetSampler(_sampler.get(), 0);
        commandList->SetConstantBuffer(_constantsBuffer.get(), 0);

        commandList->SetPrimitiveTopology(PrimitiveTopology::TRIANGLELIST);
        commandList->Draw(3, 0);
    }

    void ColorCorrectionEffect::OnImGuiRender(float deltaTime)
    {
        ImGui::Text("Color Correction (LUT)");
        ImGui::Separator();

        ImGui::SliderFloat("Strength", &_strength, 0.0f, 1.0f);

        ImGui::Spacing();
        ImGui::Text("Load LUT from file:");

        // Input text pour le chemin du fichier
        ImGui::InputText("Path", _lutPathBuffer, sizeof(_lutPathBuffer));

        if (ImGui::Button("Load LUT"))
        {
            LoadLUT(std::string(_lutPathBuffer));
        }

        ImGui::Separator();

        if (_lutTexture)
        {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "LUT Loaded: Yes");
            ImGui::Text("Size: %dx%d", _lutTexture->GetWidth(), _lutTexture->GetHeight());

            // Prévisualisation si TextureDX11 a une méthode GetNativeView ou similaire accessible via void*
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Image((ImTextureID)_lutTexture->GetRendererID(), ImVec2(256, 16));
                ImGui::EndTooltip();
            }
        }
        else
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "No LUT Texture set!");
        }
    }
} // namespace Frost