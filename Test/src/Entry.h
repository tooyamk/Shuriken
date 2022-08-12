#pragma once

#include "tests/DepthTestTester.h"

#if SRK_OS == SRK_OS_WINDOWS
#	include"tests/ExperimentTester.h"
#endif

#include "tests/CompressTextureTester.h"
#include "tests/GraphicsAdapterTester.h"
#include "tests/InputTester.h"
#include "tests/LockfreeTester.h"
#include "tests/OffscreenTester.h"
#include "tests/RenderPipelineTester.h"
#include "tests/RenderTargetTester.h"
#include "tests/VertexUpdateTester.h"
#include "tests/WindowTester.h"

//#include <immintrin.h>

class Enttry {
public:
	int32_t run() {
		/*
		__declspec(align(16)) float a[] = { 1.5, 2.5, 3.5, 4.5 };
		__declspec(align(16)) float b[] = { 1.2, 2.3, 3.4, 4.5 };
		__declspec(align(16)) float c[] = { 0.0, 0.0, 0.0, 0.0 };


		__m128 m128_a = _mm_load_ps(a);
		__m128 m128_b = _mm_load_ps(b);
		__m128 m128_c = _mm_add_ps(m128_a, m128_b);

		_mm_store_ps(c, m128_c);

		for (int i = 0; i < 4; i++) {
			printf("%f ", c[i]);
		}
		printf("\n");
		*/

		return CompressTextureTester().run();
		//return DepthTestTester().run();
		//return ExperimentTester().run();
		//return GraphicsAdapterTester().run();
		//return InputTester().run();
		//return LockfreeTester().run();
		//return OffscreenTester().run();
		//return RenderPipelineTester().run();
		//return RenderTargetTester().run();
		//return VertexUpdateTester().run();
		//return WindowTester().run();
	}
};
