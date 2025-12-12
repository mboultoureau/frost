#include "Materials/Boost.h"
#include "GameState/GameState.h"
#include "Physics/PhysicLayer.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/PhysicsSystem.h>

using namespace JPH;
using namespace Frost;
using namespace Frost::Component;

namespace GameLogic
{
    void Boost::OnCreate()
    {
        ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                              .filePath = "./assets/Prefabs/Materials/Boost/VS_Boost.hlsl" };
        ShaderDesc psDesc = { .type = ShaderType::Pixel, .filePath = "./assets/Prefabs/Materials/Boost/PS_Boost.hlsl" };

        auto vs = Shader::Create(vsDesc);
        auto ps = Shader::Create(psDesc);

        Material waveMat;
        waveMat.name = "BoostMat";
        waveMat.customVertexShader = vs;
        waveMat.customPixelShader = ps;
        waveMat.backFaceCulling = true;

        const auto& transform = GetGameObject().GetComponent<Transform>();

        _shaderParams.Time = 0.0f;
        _shaderParams.Amplitude = _waveAmplitude * 2;
        _shaderParams.Frequency = _waveFrequency;

        _impulse *= 10000;

        std::vector<uint8_t> paramData(sizeof(BoostMaterialParameters));
        memcpy(paramData.data(), &_shaderParams, sizeof(BoostMaterialParameters));
        waveMat.parameters = paramData;

        auto& staticMesh = GetGameObject().GetComponent<StaticMesh>();
        staticMesh.GetModel()->GetMaterials()[0] = std::move(waveMat);
    }

    void Boost::OnUpdate(float deltaTime)
    {
        _shaderParams.Time += deltaTime;

        if (GetGameObject().HasComponent<StaticMesh>())
        {
            auto& mesh = GetGameObject().GetComponent<StaticMesh>();
            if (mesh.GetModel() && !mesh.GetModel()->GetMaterials().empty())
            {
                std::vector<uint8_t> paramData(sizeof(BoostMaterialParameters));
                memcpy(paramData.data(), &_shaderParams, sizeof(BoostMaterialParameters));

                mesh.GetModel()->GetMaterials()[0].parameters = paramData;
            }
        }
    }

    void Boost::OnCollisionStay(Frost::BodyOnContactParameters params, float deltaTime)
    {
        auto waterBodyId = GetGameObject().GetComponent<RigidBody>().runtimeBodyID;
        auto otherBodyId = (waterBodyId == params.inBody1.GetID()) ? params.inBody2.GetID() : params.inBody1.GetID();

        auto otherLayer = Physics::GetBodyInterface().GetObjectLayer(otherBodyId);

        if (otherLayer != ObjectLayers::PLAYER)
        {
            return;
        }

        uint64_t userData = Physics::GetBodyInterface().GetUserData(otherBodyId);
        if (userData == 0)
            return;

        auto entityHandle = static_cast<entt::entity>(userData);
        GameObject playerObject(entityHandle, GetScene());

        if (!playerObject.IsValid() || !GameState::Get().IsPlayer(playerObject.GetParent()))
            return;

        BodyLockWrite body_lock(Physics::Get().physics_system.GetBodyLockInterface(), otherBodyId);
        if (!body_lock.Succeeded())
            return;

        Body& body = body_lock.GetBody();
        if (body.IsActive())
        {
            const auto& transform = GetGameObject().GetComponent<Transform>();
            body.AddForce(_impulse * Math::vector_cast<Vec3>(transform.GetForward()));
        }
    }
} // namespace GameLogic