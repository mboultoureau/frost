#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Core/UnorderedMap.h>

#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#else
	#error "JPH_DEBUG_RENDERER is not defined"
#endif

#include <string_view>

/*
* @see https://github.com/jrouwe/JoltPhysics/blob/master/TestFramework/Renderer/DebugRendererImp.h
*/

namespace Frost
{
	struct DebugRendererPhysicsConfig
	{
		bool DrawBodies = true;

		// Jolt draw settings
		JPH::BodyManager::DrawSettings BodyDrawSettings;
	};

	class DebugRendererPhysics final : public JPH::DebugRenderer
	{
	public:
		JPH_OVERRIDE_NEW_DELETE

		DebugRendererPhysics();

		virtual void	DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;
		virtual void	DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow = ECastShadow::Off) override;
		virtual Batch	CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override;
		virtual Batch	CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount) override;
		virtual void	DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) override;
		virtual void	DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight) override;


		struct Instance
		{
			/// Constructor
			Instance(JPH::Mat44Arg inModelMatrix, JPH::Mat44Arg inModelMatrixInvTrans, JPH::ColorArg inModelColor) : mModelMatrix(inModelMatrix), mModelMatrixInvTrans(inModelMatrixInvTrans), mModelColor(inModelColor) {}

			JPH::Mat44	mModelMatrix;
			JPH::Mat44	mModelMatrixInvTrans;
			JPH::Color	mModelColor;
		};

		struct InstanceWithLODInfo : public Instance
		{
			/// Constructor
			InstanceWithLODInfo(JPH::Mat44Arg inModelMatrix, JPH::Mat44Arg inModelMatrixInvTrans, JPH::ColorArg inModelColor, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq) : Instance(inModelMatrix, inModelMatrixInvTrans, inModelColor), mWorldSpaceBounds(inWorldSpaceBounds), mLODScaleSq(inLODScaleSq) {}

			/// Bounding box for culling
			JPH::AABox	mWorldSpaceBounds;

			/// Square of scale factor for LODding (1 = original, > 1 = lod out further, < 1 = lod out earlier)
			float		mLODScaleSq;
		};

		struct Instances
		{
			JPH::Array<InstanceWithLODInfo>	mInstances;

			/// Start index in mInstancesBuffer for each of the LOD in the geometry pass. Length is one longer than the number of LODs to indicate how many instances the last lod has.
			JPH::Array<int>					mGeometryStartIdx;

			/// Start index in mInstancesBuffer for each of the LOD in the light pass. Length is one longer than the number of LODs to indicate how many instances the last lod has.
			JPH::Array<int>					mLightStartIdx;
		};

		using InstanceMap = JPH::UnorderedMap<GeometryRef, Instances>;

		InstanceMap	mWireframePrimitives;
		InstanceMap	mPrimitives;
		InstanceMap	mTempPrimitives;
		InstanceMap	mPrimitivesBackFacing;
		int			mNumInstances = 0;


		JPH::Mutex	mPrimitivesLock;


		void Draw();
		void DrawTriangles();
	};
}
