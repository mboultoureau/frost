#include "Water.h"

#include "Frost.h"

#include "../Game.h"
#include "../Physics/PhysicsLayer.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Player/Vehicles/Boat.h"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

void
WaterScript::OnCollisionEnter(BodyOnContactParameters params, float deltaTime)
{
    auto id1 = params.inBody1.GetID();
    auto id2 = params.inBody2.GetID();
    if (Boat::IsBoatID(id1))
        Boat::SetBoatWater(id1, _water);
    if (Boat::IsBoatID(id2))
        Boat::SetBoatWater(id2, _water);
}

void
WaterScript::OnCollisionStay(BodyOnContactParameters params, float deltaTime)
{
    using namespace JPH;

    auto id = (_water->GetBodyId() == params.inBody1.GetID()) ? params.inBody2.GetID() : params.inBody1.GetID();

    auto layer1 = Physics::GetBodyInterface().GetObjectLayer(params.inBody1.GetID());
    auto layer2 = Physics::GetBodyInterface().GetObjectLayer(params.inBody2.GetID());

    if (layer1 != ObjectLayers::PLAYER && layer2 != ObjectLayers::PLAYER)
    {
        return;
    }

    BodyLockWrite body_lock(Physics::Get().physics_system.GetBodyLockInterface(), id);
    Body& body = body_lock.GetBody();
    if (body.IsActive())
    {
        // Use center of mass position to determine water surface position (you
        // could test multiple points on the actual shape of the boat to get a
        // more accurate result)
        RVec3 surface_position = _water->GetWaterSurfacePosition(body.GetCenterOfMassPosition());

        // Crude way of approximating the surface normal
        RVec3 p2 = _water->GetWaterSurfacePosition(body.GetCenterOfMassPosition() + Vec3(0, 0, 1));
        RVec3 p3 = _water->GetWaterSurfacePosition(body.GetCenterOfMassPosition() + Vec3(1, 0, 0));
        Vec3 surface_normal = Vec3(p2 - surface_position).Cross(Vec3(p3 - surface_position)).Normalized();

        // Determine buoyancy and drag
        float buoyancy, linear_drag, angular_drag;
        if (Boat::IsBoatID(id))
        {
            buoyancy = cBoatBuoyancy;
            linear_drag = cBoatLinearDrag;
            angular_drag = cBoatAngularDrag;
        }
        else
        {
            buoyancy = cBarrelBuoyancy;
            linear_drag = cBarrelLinearDrag;
            angular_drag = cBarrelAngularDrag;
        }

        // Apply buoyancy to the body
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

void
WaterScript::OnUpdate(float deltaTime)
{
    _water->UpdateShader(deltaTime);
    Physics::ActivateBody(_water->GetBodyId());

#if FT_DEBUG
    using namespace JPH;
    auto tr = GetGameObject().GetComponent<WorldTransform>();
    // Draw the water surface
    const float step = 1.0f;
    for (float z = tr.position.z; z < tr.position.z + tr.scale.z; z += step)
    {
        RVec3 p1 = _water->GetWaterSurfacePosition(RVec3(tr.position.x, 0, z));
        RVec3 p2 = _water->GetWaterSurfacePosition(RVec3(tr.position.x, 0, z + step));
        RVec3 p3 = _water->GetWaterSurfacePosition(RVec3(tr.position.x + tr.scale.x, 0, z));
        RVec3 p4 = _water->GetWaterSurfacePosition(RVec3(tr.position.x + tr.scale.x, 0, z + step));
        Physics::GetDebugRenderer()->DrawTriangle(p1, p2, p3, Color::sWhite);
        Physics::GetDebugRenderer()->DrawTriangle(p2, p4, p3, Color::sWhite);
    }
#endif
}

JPH::RVec3
Water::GetWaterSurfacePosition(JPH::RVec3Arg inXZPosition) const
{
    using namespace std::chrono;
    using namespace JPH;
    auto timePoint = Timer::Clock::now();
    auto mTime = time_point_cast<duration<double>>(timePoint);
    float t = mTime.time_since_epoch().count();
    return RVec3(inXZPosition.GetX(),
                 cMinWaterHeight + Sin(waveLength * float(inXZPosition.GetZ()) + waveFrequency * t) *
                                       (cMaxWaterHeight - cMinWaterHeight),
                 inXZPosition.GetZ());
}

void
Water::_SetClosestPlayerPosToShader()
{
    auto pos = _water.GetComponent<WorldTransform>().position;
    auto player = Player::GetClosestPlayer(pos);
    if (player)
    {
        auto playerPos = player->GetPlayerID().GetComponent<WorldTransform>().position;
        _shaderParams.PlayerPosition[0] = playerPos.x;
        _shaderParams.PlayerPosition[1] = playerPos.y;
        _shaderParams.PlayerPosition[2] = playerPos.z;
    }
    else
    {
        _shaderParams.PlayerPosition[0] = 0.0f;
        _shaderParams.PlayerPosition[1] = 0.0f;
        _shaderParams.PlayerPosition[2] = 0.0f;
    }
}
void
Water::UpdateShader(float deltaTime)
{
    _shaderParams.Time += deltaTime;

    if (_water.HasComponent<StaticMesh>())
    {
        auto& mesh = _water.GetComponent<StaticMesh>();
        if (mesh.GetModel() && !mesh.GetModel()->GetMaterials().empty())
        {
            std::vector<uint8_t> paramData(sizeof(WaterMaterialParameters));
            memcpy(paramData.data(), &_shaderParams, sizeof(WaterMaterialParameters));

            mesh.GetModel()->GetMaterials()[0].parameters = paramData;
        }
    }
}

Water::Water(Vector3 pos, EulerAngles rot, Vector3 scale, float waveAmplitude)
{
    using namespace JPH;
    Scene& scene = Game::GetScene();

    // Create game object -----------------------
    _water = scene.CreateGameObject("Water");
    _water.AddComponent<Transform>(pos, rot, scale);
    auto& cubeModel = _water.AddComponent<StaticMesh>(MeshSourceCube{ 1.0f, scale * 0.5f });

    // Create water sensor. We use this to detect which bodies entered the water (in this sample we could have assumed
    // everything is in the water)
    auto offset = Frost::Math::vector_cast<Frost::Math::Vector3>(4 * waveAmplitude * Vec3(0, 1, 0));
    RigidBody rbComp{ ShapeBox{ Vector3{ scale * 0.5f + offset } },
                      ObjectLayers::WATER,
                      RigidBody::MotionType::Static };
    rbComp.isSensor = true;
    auto& rb = _water.AddComponent<RigidBody>(rbComp);
    _bodyId = _water.GetComponent<RigidBody>().runtimeBodyID;
    _water.AddScript<WaterScript>(this);

    cMinWaterHeight = pos.y + scale.y - waveAmplitude / 2.0f;
    cMaxWaterHeight = pos.y + scale.y + waveAmplitude / 2.0f;

    // Create shader -----------------------

    ShaderDesc vsDesc = { .type = ShaderType::Vertex, .filePath = "../Game/resources/shaders/Water/VS_Water.hlsl" };
    ShaderDesc hsDesc = { .type = ShaderType::Hull, .filePath = "../Game/resources/shaders/Water/HS_Water.hlsl" };
    ShaderDesc dsDesc = { .type = ShaderType::Domain, .filePath = "../Game/resources/shaders/Water/DS_Water.hlsl" };
    ShaderDesc psDesc = { .type = ShaderType::Pixel, .filePath = "../Game/resources/shaders/Water/PS_Water.hlsl" };

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
    _shaderParams.TopAmplitude = waveAmplitude * 2;
    _shaderParams.TopFrequency = waveFrequency;
    _shaderParams.TopWaveLength = waveLength;
    _shaderParams.TessellationFactor = 32.0f;
    _shaderParams.BevelSize = 0.0001f;
    _SetClosestPlayerPosToShader();

    std::vector<uint8_t> paramData(sizeof(WaterMaterialParameters));
    memcpy(paramData.data(), &_shaderParams, sizeof(WaterMaterialParameters));
    waveMat.parameters = paramData;

    cubeModel.GetModel()->GetMaterials()[0] = std::move(waveMat);

    cMinWaterHeight = pos.y + scale.y / 2 - waveAmplitude / 2.0f;
    cMaxWaterHeight = pos.y + scale.y / 2 + waveAmplitude / 2.0f;
}