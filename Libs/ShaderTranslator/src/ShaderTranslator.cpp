#include "ShaderTranslator.h"
#include <vector>

namespace shader_translator {
	ShaderTranslator::ShaderTranslator() :
		_dxcll(nullptr),
		_dxCreateInsFn(nullptr),
		_lib(nullptr),
		_compiler(nullptr) {
		_dxcll = LoadLibraryA("dxcompiler.dll");
		if (_dxcll) {
			_dxCreateInsFn = (DxcCreateInstanceProc)GetProcAddress(_dxcll, "DxcCreateInstance");
			if (_dxCreateInsFn) {
				IFT(_dxCreateInsFn(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void**)(&_lib)));
				IFT(_dxCreateInsFn(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void**)(&_compiler)));
			}
		}
	}

	void ShaderTranslator::translate(char* src) {
		std::wstring shaderProfile;
		shaderProfile = L"vs";
		shaderProfile.push_back(L'_');
		shaderProfile.push_back(L'0' + '4');
		shaderProfile.push_back(L'_');
		shaderProfile.push_back(L'0' + '0');

		std::vector<DxcDefine> dxcDefines;

		CComPtr<IDxcBlobEncoding> sourceBlob;
		IFT(_lib->CreateBlobWithEncodingOnHeapCopy(src, static_cast<UINT32>(strlen(src)), CP_UTF8, &sourceBlob));
		IFTARG(sourceBlob->GetBufferSize() >= 4);

		/*
		std::wstring shaderNameUtf16;
		Unicode::UTF8ToUTF16String("fileName", &shaderNameUtf16);

		std::wstring entryPointUtf16;
		Unicode::UTF8ToUTF16String("main", &entryPointUtf16);
		*/

		std::vector<std::wstring> dxcArgStrings;
		dxcArgStrings.push_back(L"-Od");
		dxcArgStrings.push_back(L"-spirv");

		std::vector<const wchar_t*> dxcArgs;
		dxcArgs.reserve(dxcArgStrings.size());
		for (const auto& arg : dxcArgStrings) dxcArgs.push_back(arg.c_str());

		//CComPtr<IDxcIncludeHandler> includeHandler = new ScIncludeHandler(std::move(source.loadIncludeCallback));
		CComPtr<IDxcOperationResult> compileResult;
		IFT(_compiler->Compile(sourceBlob, L"shaderName", L"main", shaderProfile.c_str(),
			dxcArgs.data(), (UINT32)(dxcArgs.size()), dxcDefines.data(),
			(UINT32)(dxcDefines.size()), nullptr, &compileResult));

		HRESULT status;
		IFT(compileResult->GetStatus(&status));

		CComPtr<IDxcBlobEncoding> errors;
		IFT(compileResult->GetErrorBuffer(&errors));
		if (errors != nullptr) {
			if (errors->GetBufferSize() > 0) {
				int a = 1;
				//ret.errorWarningMsg = CreateBlob(errors->GetBufferPointer(), static_cast<uint32_t>(errors->GetBufferSize()));
			}
			errors = nullptr;
		}

		if (SUCCEEDED(status)) {
			CComPtr<IDxcBlob> program;
			IFT(compileResult->GetResult(&program));
			compileResult = nullptr;
			if (program != nullptr) {
				//ret.target = CreateBlob(program->GetBufferPointer(), static_cast<uint32_t>(program->GetBufferSize()));
				//ret.hasError = false;
				int a = 1;
			}
		}

		int a = 1;
	}
}



#include <fstream>

char* readFile(char* path) {
	char* data = nullptr;
	std::ifstream stream(path, std::ios::in | std::ios::binary);
	if (stream.good()) {
		auto beg = stream.tellg();
		stream.seekg(0, std::ios::end);
		auto end = stream.tellg();
		auto size = end - beg;

		data = new char[size + 1];
		data[size] = 0;

		stream.seekg(0, std::ios::beg);
		stream.read(data, size);
	}
	stream.close();
	return data;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	auto data = readFile("../../Resources/vert.hlsl");

	shader_translator::ShaderTranslator st;
	st.translate(data);

	return 0;
}