#include "tests/DepthTestTester.h"
#include "tests/InputTester.h"
#include "tests/RenderPipelineTester.h"
#include "tests/RenderTargetTester.h"
#include "tests/VertexUpdateTester.h"

/*
int main() {
	createWindow();
	std::cout << "Hello World!\n";
}
*/

#include <unordered_map>
#include <vector>
#include <functional>
#include <atomic>
#include <thread>
#include <assert.h>
#include <mutex>
#include <any>
#include <fstream>
#include <bitset>

//#include <version>
#include <type_traits>
//#include <bit>

#include <any>
#include <immintrin.h>
//#include <type_traits>

//int main(int argc, char* argv[]) {
//	HMODULE HIn = GetModuleHandle(NULL);
//	FreeConsole();
int32_t WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
//#pragma comment(linker, "/subsystem:console")
//int32_t main() {
#if AE_OS == AE_OS_WIN
	SetDllDirectoryW((getAppPath().parent_path().wstring() + L"/libs/").data());
#endif
	/*
	abc
	*/

	//new EventListener11(1);

	//println(String::toString(hash::CRC::calc<64>((uint8_t*)strr.data(), strr.size(), 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, true, true, hash::CRC::createTable<64>(0x42F0E1EBA9EA3693ULL)), 16));
	

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
	//return (new InputTester())->run();
	//return (new RenderPipelineTester())->run();
	return (new RenderTargetTester())->run();
	//return (new VertexUpdateTester())->run();
}