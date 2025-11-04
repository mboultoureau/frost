#include "Frost/Physics/DebugRendererPhysics.h"

#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#else
	#error "JPH_DEBUG_RENDERER is not defined"
#endif

#include "Frost/Debugging/Logger.h"

namespace Frost
{
	using namespace JPH;

	DebugRendererPhysics::DebugRendererPhysics()
	{
		DebugRenderer::Initialize();
	}

	void DebugRendererPhysics::DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor)
	{
	}

	void DebugRendererPhysics::DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor, ECastShadow inCastShadow)
	{
		FT_ENGINE_INFO("DrawTriangle called in DebugRendererPhysics - this is a stub implementation");
	}

	DebugRenderer::Batch DebugRendererPhysics::CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount)
	{
		return Batch();
	}

	DebugRenderer::Batch DebugRendererPhysics::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const uint32* inIndices, int inIndexCount)
	{
		return Batch();
	}

	void DebugRendererPhysics::DrawGeometry(RMat44Arg inModelMatrix, const AABox& inWorldSpaceBounds, float inLODScaleSq, ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode)
	{
		lock_guard lock(mPrimitivesLock);

		// Change this value at big distances
		RVec3 offset = RVec3::sZero();

		Mat44 model_matrix = inModelMatrix.PostTranslated(-offset).ToMat44();
		AABox world_space_bounds = inWorldSpaceBounds;
		world_space_bounds.Translate(Vec3(-offset));

		// Our pixel shader uses alpha only to turn on/off shadows
		Color color = inCastShadow == ECastShadow::On ? Color(inModelColor, 255) : Color(inModelColor, 0);

		if (inDrawMode == EDrawMode::Wireframe)
		{
			mWireframePrimitives[inGeometry].mInstances.push_back({ model_matrix, model_matrix.GetDirectionPreservingMatrix(), color, world_space_bounds, inLODScaleSq });
			++mNumInstances;
		}
		else
		{
			if (inCullMode != ECullMode::CullFrontFace)
			{
				mPrimitives[inGeometry].mInstances.push_back({ model_matrix, model_matrix.GetDirectionPreservingMatrix(), color, world_space_bounds, inLODScaleSq });
				++mNumInstances;
			}

			if (inCullMode != ECullMode::CullBackFace)
			{
				mPrimitivesBackFacing[inGeometry].mInstances.push_back({ model_matrix, model_matrix.GetDirectionPreservingMatrix(), color, world_space_bounds, inLODScaleSq });
				++mNumInstances;
			}
		}
	}

	void DebugRendererPhysics::DrawText3D(RVec3Arg inPosition, const string_view& inString, ColorArg inColor, float inHeight)
	{
	}

	void DebugRendererPhysics::DrawTriangles()
	{

		/*
		// Bind the shadow map texture
		mRenderer->GetShadowMap()->Bind();

		if (!mPrimitives.empty() || !mTempPrimitives.empty())
		{
			// Bind the normal shader, back face culling
			mTriangleStateBF->Activate();

			// Draw all primitives
			if (mNumInstances > 0)
				for (InstanceMap::value_type& v : mPrimitives)
					DrawInstances(v.first, v.second.mGeometryStartIdx);
			for (InstanceMap::value_type& v : mTempPrimitives)
				DrawInstances(v.first, v.second.mGeometryStartIdx);
		}

		if (!mPrimitivesBackFacing.empty())
		{
			// Front face culling, the next batch needs to render inside out
			mTriangleStateFF->Activate();

			// Draw all back primitives
			for (InstanceMap::value_type& v : mPrimitivesBackFacing)
				DrawInstances(v.first, v.second.mGeometryStartIdx);
		}

		if (!mWireframePrimitives.empty())
		{
			// Wire frame mode
			mTriangleStateWire->Activate();

			// Draw all wireframe primitives
			for (InstanceMap::value_type& v : mWireframePrimitives)
				DrawInstances(v.first, v.second.mGeometryStartIdx);
		}
		*/
	}

	void DebugRendererPhysics::Draw()
	{
		// DrawLines();
		// DrawTriangles();
		// DrawTexts();
	}
}