#include "tests/DepthTestTester.h"

#if AE_OS == AE_OS_WINDOWS
#	include"tests/ExperimentTester.h"
#endif

#include "tests/GraphicsAdapterTester.h"
#include "tests/InputTester.h"
#include "tests/LockfreeTester.h"
#include "tests/OffscreenTester.h"
#include "tests/RenderPipelineTester.h"
#include "tests/RenderTargetTester.h"
#include "tests/VertexUpdateTester.h"
#include "tests/WindowTester.h"

#include <immintrin.h>

int32_t run() {
#if AE_OS == AE_OS_WINDOWS
	SetDllDirectoryW((getAppPath().parent_path().wstring() + L"/libs/").data());
#endif
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

	//return (new DepthTestTester())->run();
	//return (new ExperimentTester())->run();
	//return (new GraphicsAdapterTester())->run();
	//return (new InputTester())->run();
	//return (new LockfreeTester())->run();
	//return (new OffscreenTester())->run();
	return (new RenderPipelineTester())->run();
	//return (new RenderTargetTester())->run();
	//return (new VertexUpdateTester())->run();
	//return (new WindowTester())->run();
}

#if AE_OS == AE_OS_WINDOWS
//#pragma comment(linker, "/subsystem:console")
int32_t WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int32_t nCmdShow) {
	return run();
}
#endif

int32_t main() {
	return run();
}