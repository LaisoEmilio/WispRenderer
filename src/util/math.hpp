#pragma once

#include <DirectXMath.h>

namespace util
{

	inline DirectX::XMMATRIX StripTranslationAndLowerVector(DirectX::XMMATRIX matrix)
	{
		DirectX::XMMATRIX upper3x3 = matrix;
		upper3x3.r[0].m128_f32[3] = 0.f;
		upper3x3.r[1].m128_f32[3] = 0.f;
		upper3x3.r[2].m128_f32[3] = 0.f;
		upper3x3.r[3].m128_f32[0] = 0.f;
		upper3x3.r[3].m128_f32[1] = 0.f;
		upper3x3.r[3].m128_f32[2] = 0.f;
		upper3x3.r[3].m128_f32[3] = 1.f;

		return upper3x3;
	}

} /* util */