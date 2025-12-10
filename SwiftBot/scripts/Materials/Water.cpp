#include "Materials/Water.h"
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
    void Water::OnCreate()
    {
        ShaderDesc vsDesc = { .type = ShaderType::Vertex,
                              .filePath = "./assets/Prefabs/Materials/Water/VS_Water.hlsl" };
        ShaderDesc hsDesc = { .type = ShaderType::Hull, .filePath = "./assets/Prefabs/Materials/Water/HS_Water.hlsl" };
        ShaderDesc dsDesc = { .type = ShaderType::Domain,
                              .filePath = "./assets/Prefabs/Materials/Water/DS_Water.hlsl" };
        ShaderDesc psDesc = { .type = ShaderType::Pixel, .filePath = "./assets/Prefabs/Materials/Water/PS_Water.hlsl" };

        auto vs = Shader::Create(vsDesc);
        auto hs = Shader::Create(hsDesc);
        auto ds = Shader::Create(dsDesc);
        auto ps = Shader::Create(psDesc);

        Material waveMat;
        waveMat.name = "TessellatedWaves";
        waveMat.customVertexShader = vs;
        waveMat.hullShader = hs;
        waveMat.domainShader = ds;
        waveMat.customPixelShader = ps;
        waveMat.backFaceCulling = false;

        _shaderParams.Time = 0.0f;
        _shaderParams.TopAmplitude = _waveAmplitude * 2;
        _shaderParams.TopFrequency = _waveFrequency;
        _shaderParams.TopWaveLength = _waveLength;
        _shaderParams.TessellationFactor = 32.0f;
        _shaderParams.BevelSize = 0.0001f;

        std::vector<uint8_t> paramData(sizeof(WaterMaterialParameters));
        memcpy(paramData.data(), &_shaderParams, sizeof(WaterMaterialParameters));
        waveMat.parameters = paramData;

        auto& staticMesh = GetGameObject().GetComponent<StaticMesh>();
        staticMesh.GetModel()->GetMaterials()[0] = std::move(waveMat);

        const auto& transform = GetGameObject().GetComponent<Transform>();

        _minWaterHeight = transform.position.y + transform.scale.y / 2 - _waveAmplitude / 2.0f;
        _maxWaterHeight = transform.position.y + transform.scale.y / 2 + _waveAmplitude / 2.0f;
    }

    void Water::OnUpdate(float deltaTime)
    {
        _shaderParams.Time += deltaTime;

        if (GetGameObject().HasComponent<StaticMesh>())
        {
            auto& mesh = GetGameObject().GetComponent<StaticMesh>();
            if (mesh.GetModel() && !mesh.GetModel()->GetMaterials().empty())
            {
                std::vector<uint8_t> paramData(sizeof(WaterMaterialParameters));
                memcpy(paramData.data(), &_shaderParams, sizeof(WaterMaterialParameters));

                mesh.GetModel()->GetMaterials()[0].parameters = paramData;
            }
        }
    }

    JPH::RVec3 Water::GetWaterSurfacePosition(JPH::RVec3Arg inXZPosition) const
    {
        using namespace std::chrono;
        auto timePoint = steady_clock::now();
        auto mTime = time_point_cast<duration<double>>(timePoint);
        float t = mTime.time_since_epoch().count();
        return RVec3(inXZPosition.GetX(),
                     _minWaterHeight + Sin(_waveLength * float(inXZPosition.GetZ()) + _waveFrequency * t) *
                                           (_maxWaterHeight - _minWaterHeight),
                     inXZPosition.GetZ());
    }

    void Water::OnCollisionEnter(Frost::BodyOnContactParameters params, float deltaTime)
    {
        auto bodyId1 = params.inBody1.GetID();
        auto bodyId2 = params.inBody2.GetID();

        auto otherBodyId = (GetGameObject().GetComponent<RigidBody>().runtimeBodyID == bodyId1) ? bodyId2 : bodyId1;

        uint64_t userData = Physics::GetBodyInterface().GetUserData(otherBodyId);
        if (userData == 0)
            return;

        auto entityHandle = static_cast<entt::entity>(userData);
        GameObject playerObject(entityHandle, GetScene());

        if (playerObject.IsValid() && GameState::Get().IsPlayer(playerObject.GetParent()))
        {
            GameState::Get().GetPlayerData(playerObject.GetParent()).isInWater = true;
        }
    }

    void Water::OnCollisionStay(Frost::BodyOnContactParameters params, float deltaTime)
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
            RVec3 surface_position = GetWaterSurfacePosition(body.GetCenterOfMassPosition());

            RVec3 p2 = GetWaterSurfacePosition(body.GetCenterOfMassPosition() + Vec3(0, 0, 1));
            RVec3 p3 = GetWaterSurfacePosition(body.GetCenterOfMassPosition() + Vec3(1, 0, 0));
            Vec3 surface_normal = Vec3(p2 - surface_position).Cross(Vec3(p3 - surface_position)).Normalized();

            float buoyancy, linear_drag, angular_drag;

            const auto& playerData = GameState::Get().GetPlayerData(playerObject.GetParent());
            if (playerData.currentVehicle == VehicleType::BOAT)
            {
                buoyancy = cBoatBuoyancy;
                linear_drag = cBoatLinearDrag;
                angular_drag = cBoatAngularDrag;
            }
            else
            {
                buoyancy = cDefaultBuoyancy;
                linear_drag = cDefaultLinearDrag;
                angular_drag = cDefaultAngularDrag;
            }

            body.ApplyBuoyancyImpulse(surface_position,
                                      surface_normal,
                                      buoyancy,
                                      linear_drag,
                                      angular_drag,
                                      Vec3::sZero(),
                                      Physics::Get().physics_system.GetGravity(),
                                      deltaTime);
        }
    }

    void Water::OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime)
    {
        auto waterHandle = GetGameObject().GetHandle();
        auto otherHandle = (params.first == waterHandle) ? params.second : params.first;

        GameObject otherObject = GetScene()->GetGameObjectFromId(otherHandle);
        if (otherObject.IsValid() && GameState::Get().IsPlayer(otherObject.GetParent()))
        {
            GameState::Get().GetPlayerData(otherObject.GetParent()).isInWater = false;
        }
    }
} // namespace GameLogic