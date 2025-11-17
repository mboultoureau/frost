#include "Frost/Physics/ShapeFactory.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Utils/Math/Vector.h"

#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>

using namespace Frost::Math;

namespace Frost
{
	JPH::ShapeRefC ShapeFactory::CreateHeightMap(const HeightMapConfig& config)
	{
		FT_ENGINE_ASSERT(config.texture, "ShapeFactory: HeightMapConfig texture is null.");
		const uint32_t textureWidth = config.texture->GetWidth();
		const uint32_t textureHeight = config.texture->GetHeight();
		const uint32_t channels = config.texture->GetChannels();

		FT_ENGINE_ASSERT(textureWidth > 0 && textureHeight > 0, "ShapeFactory: Invalid texture size.");

		std::vector<uint8_t> pixels = config.texture->GetData();
		FT_ENGINE_ASSERT(!pixels.empty(), "ShapeFactory: Texture data is empty.");

		uint32_t sampleCount = textureWidth > textureHeight ? textureWidth : textureHeight;
		if (sampleCount < 2) sampleCount = 2;

		std::vector<float> samples(sampleCount * sampleCount, 0.0f);

		for (uint32_t y = 0; y < (uint32_t)textureHeight; ++y)
		{
			for (uint32_t x = 0; x < (uint32_t)textureWidth; ++x)
			{
				size_t pixIdx = (y * textureWidth + x) * channels;
				uint8_t value = pixels[pixIdx];
				
				// Normalise en 0..1
				float sample = float(value) / 255.0f;
				samples[y * sampleCount + x] = sample;
			}
		}

		Vector3 offset(-0.5f * config.scale.width, 0.0f, -0.5f * config.scale.depth);
		Vector3 scale(
			1.0f / float(sampleCount) * config.scale.width,
			config.scale.height * 10.0f,
			1.0f / float(sampleCount) * config.scale.depth
		);

		JPH::HeightFieldShapeSettings settings { samples.data(), vector_cast<JPH::Vec3>(offset), vector_cast<JPH::Vec3>(scale), sampleCount};

		JPH::Shape::ShapeResult result = settings.Create();
		if (!result.IsValid())
		{
			FT_ENGINE_ERROR("Failed to create HeightFieldShape from texture");
			return nullptr;
		}

		JPH::RefConst<JPH::Shape> shape = result.Get();
		return shape;

	}
}