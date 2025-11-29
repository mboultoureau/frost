#pragma once

#include "Frost/Asset/Texture.h"
#include "Frost/Utils/Math/Matrix.h"

namespace Frost
{
    class PostEffect
    {
    public:
        virtual ~PostEffect() = default;

        virtual void OnPreRender(float deltaTime, Math::Matrix4x4& viewMatrix, Math::Matrix4x4& projectionMatrix) {};
        virtual void OnPostRender(float deltaTime, CommandList* commandList, Texture* source, Texture* destination) {};
        virtual void OnImGuiRender(float deltaTime) {};
        virtual const char* GetName() const { return "PostEffect"; }
        bool IsEnabled() const { return _enabled; }
        void SetEnabled(bool enabled) { _enabled = enabled; }
        virtual bool IsPostProcessingPass() const { return true; }
        void SetNormalTexture(Texture* normalTexture) { _normal = normalTexture; };
        void SetMaterialTexture(Texture* materialTexture) { _material = materialTexture; };
        void SetDepthTexture(Texture* depthTexture) { _depth = depthTexture; };
        void SetCustom0Texture(Texture* customTexture) { _custom0 = customTexture; };
        void SetCustom1Texture(Texture* customTexture) { _custom1 = customTexture; };
        void SetCustom2Texture(Texture* customTexture) { _custom2 = customTexture; };
        void SetCustom3Texture(Texture* customTexture) { _custom3 = customTexture; };

    protected:
        bool _enabled = true;

        // Texture* _albedo; //OnPostRender handles this one
        Texture* _normal;
        Texture* _material;
        Texture* _depth;
        Texture* _custom0;
        Texture* _custom1;
        Texture* _custom2;
        Texture* _custom3;
    };
} // namespace Frost
