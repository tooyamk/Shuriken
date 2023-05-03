#pragma once

#include "../BaseTester.h"

inline int _aaaid = 0;

class AAA {
public:
	AAA() {
		_id = ++_aaaid;
		_val = 123;
	}

	AAA(const AAA& v) {
		_id = ++_aaaid;
		_val = v._val;
	}

	AAA(AAA&& v) {
		_id = ++_aaaid;
		_val = v._val;
	}

	~AAA() {
		_val = -1;
	}

	int _val = 0;
	int _id;
};

class GraphicsTester : public BaseTester {
public:
	virtual int32_t SRK_CALL run() override {
		AAA aaab;
		auto tp = new ThreadPool(10);
		{
			//AAA aaa;
			tp->enqueue([](AAA& a) {
				int z = 1;
				a._val++;
				}, std::ref(aaab));
		}

		do {
			std::this_thread::yield();
		} while (true);

		auto monitors = Monitor::getMonitors();
		auto vms = monitors[0].getVideoModes();

		auto aaaa = sizeof(SamplerAddress);
		
		IntrusivePtr wml = new WindowModuleLoader();
		if (!wml->load(getWindowDllPath())) return 0;

		auto wm = wml->create();
		if (!wm) return 0;

		CreateWindowDescriptor desc;
		desc.style.resizable = true;
		desc.contentSize.set(800, 600);
		auto win = wm->crerate(desc);
		if (!win) return 0;

		IntrusivePtr gml = new GraphicsModuleLoader();

		//if (!gml->load(getDllPath("srk-module-graphics-d3d11"))) return 0;
		//if (!gml->load(getDllPath("srk-module-graphics-d3d12"))) return 0;
		//if (!gml->load(getDllPath("srk-module-graphics-gl"))) return 0;
		if (!gml->load(getDllPath("srk-module-graphics-vulkan"))) return 0;

		CreateGrahpicsModuleDescriptor createGrahpicsModuleDesc;
		createGrahpicsModuleDesc.window = win;
		createGrahpicsModuleDesc.sampleCount = 4;
		createGrahpicsModuleDesc.debug = Environment::IS_DEBUG;

		auto graphics = gml->create(createGrahpicsModuleDesc);
		if (!graphics) return 0;

		graphics->getEventDispatcher()->addEventListener(GraphicsEvent::ERR, createEventListener<GraphicsEvent>([](Event<GraphicsEvent>& e) {
			printaln(*(std::string_view*)e.getData());
		int a = 1;
			}));

		IntrusivePtr looper = new Looper(1000.0 / 60.0);

		printaln(L"================================"sv);

		printaln(L"Graphics Version : "sv, graphics->getVersion());

		{
			auto bs = graphics->createBlendState();
			printaln(L"createBlendState : "sv, bs ? L"succeed"sv : L"failed"sv);

			auto dss = graphics->createDepthStencilState();
			printaln(L"createDepthStencilState : "sv, dss ? L"succeed"sv : L"failed"sv);

			auto rs = graphics->createRasterizerState();
			printaln(L"createRasterizerState : "sv, rs ? L"succeed"sv : L"failed"sv);

			auto vb = graphics->createVertexBuffer();
			printaln(L"createVertexBuffer : "sv, vb ? L"succeed"sv : L"failed"sv);
			if (vb) {
				auto rst = vb->create(12, Usage::MAP_READ_WRITE, Usage::NONE);
				printaln(L"VertexBuffer::create(rw) : "sv, rst ? L"succeed"sv : L"failed"sv);
				if (rst) {
					auto usage = vb->map(Usage::MAP_WRITE);
					printaln(L"VertexBuffer::map(rw-w) : "sv, usage != Usage::NONE ? L"succeed"sv : L"failed"sv);
					float32_t wbuf[] = { 1.0f, 2.0f, 3.0f };
					constexpr auto n = std::extent_v<decltype(wbuf)>;
					auto wsize = vb->write(wbuf, sizeof(wbuf), 0);
					printaln(L"VertexBuffer::write : "sv, wsize != -1 ? L"succeed"sv : L"failed"sv);
					vb->unmap();

					float32_t rbuf[n];
					usage = vb->map(Usage::MAP_READ);
					printaln(L"VertexBuffer::map(rw-r) : "sv, usage != Usage::NONE ? L"succeed"sv : L"failed"sv);
					auto rsize = vb->read(rbuf, sizeof(rbuf), 0);
					vb->unmap();
					auto isSame = rsize != -1 && wsize == rsize;
					for (size_t i = 0; i < n; ++i) {
						if (wbuf[i] != rbuf[i]) {
							isSame = false;
							break;
						}
					}
					printaln(L"VertexBuffer::read : "sv, isSame ? L"succeed"sv : L"failed"sv);

					vb->create(12, Usage::UPDATE, Usage::NONE);
					auto upsize = vb->update(wbuf, sizeof(wbuf), 0);
					printaln(L"VertexBuffer::update(rw) : "sv, upsize != -1 && upsize == sizeof(wbuf) ? L"succeed"sv : L"failed"sv);

					vb->create(12, Usage::COPY_SRC, Usage::NONE, wbuf, sizeof(wbuf));

					auto vb2 = graphics->createVertexBuffer();
					printaln(L"createVertexBuffer : "sv, vb2 ? L"succeed"sv : L"failed"sv);
					if (vb2) {
						rst = vb2->create(12, Usage::COPY_DST | Usage::MAP_READ, Usage::NONE);
						printaln(L"VertexBuffer::create(dr) : "sv, rst ? L"succeed"sv : L"failed"sv);
						if (rst) {
							auto cpysize = vb2->copyFrom(0, vb, Box1uz(Vec1uz(0), Vec1uz(vb->getSize())));
							printaln(L"VertexBuffer::copyFrom : "sv, cpysize != -1 ? L"succeed"sv : L"failed"sv);
							usage = vb2->map(Usage::MAP_READ);
							printaln(L"VertexBuffer::map(dr-r) : "sv, usage != Usage::NONE ? L"succeed"sv : L"failed"sv);
							rsize = vb2->read(rbuf, sizeof(rbuf), 0);
							vb2->unmap();
							isSame = cpysize != -1 && rsize != -1 && wsize == cpysize && wsize == rsize;
							for (size_t i = 0; i < n; ++i) {
								if (wbuf[i] != rbuf[i]) {
									isSame = false;
									break;
								}
							}
							printaln(L"VertexBuffer::copyFrom data check : "sv, isSame ? L"succeed"sv : L"failed"sv);
						}
					}
				}
			}

			auto tr = graphics->createTexture2DResource();
			printaln(L"createTexture2DResource : "sv, tr ? L"succeed"sv : L"failed"sv);
			if (tr) {
				constexpr TextureFormat texFmt = TextureFormat::R8G8B8A8_UNORM;
				constexpr Vec2uz texDim(2, 2);

				auto mipLevels = TextureUtility::getMipLevels(texDim);
				size_t totalBytes;
				std::vector<size_t> mipBytes(mipLevels);
				TextureUtility::getMipsInfo(texFmt, texDim, mipLevels, &totalBytes, mipBytes.data());
				std::vector<uint8_t> texData(totalBytes);
				std::vector<void*> mipData(mipLevels);
				size_t offset = 0;
				for (size_t i = 0; i < mipLevels; ++i) {
					mipData[i] = texData.data() + offset;
					
					for (size_t j = 0; j < mipBytes[i]; ++j) {
						texData.data()[offset + j] = 255 / (i + 1);
					}

					offset += mipBytes[i];
				}

				auto rst = tr->create(texDim, 0, mipLevels, 1, texFmt, Usage::MAP_WRITE | Usage::COPY_SRC, Usage::NONE, mipData.data());
				printaln(L"Texture2DResource::create(u) : "sv, rst ? L"succeed"sv : L"failed"sv);
				if (!rst) {
					rst = tr->create(texDim, 0, 1, 1, texFmt, Usage::MAP_WRITE | Usage::COPY_SRC, Usage::NONE, mipData.data());
					printaln(L"Texture2DResource::create(u) : "sv, rst ? L"succeed"sv : L"failed"sv);
				}
				if (rst) {
					std::vector<uint8_t>* updateOrWriteData = nullptr;
					std::vector<uint8_t> updateData(TextureUtility::getPerBlockBytes(texFmt));
					for (size_t i = 0; i < updateData.size(); ++i) updateData.data()[i] = i;
					if ((tr->getUsage() & Usage::UPDATE) == Usage::UPDATE) {
						updateOrWriteData = &updateData;
						rst = tr->update(0, 0, Box2uz(Vec2uz(0, 0), Vec2uz(1, 1)), updateData.data());
						printaln(L"Texture2DResource::update : "sv, rst ? L"succeed"sv : L"failed"sv);
					}

					std::vector<uint8_t> writeData(TextureUtility::getPerBlockBytes(texFmt));
					for (size_t i = 0; i < writeData.size(); ++i) writeData.data()[i] = i;
					if ((tr->getUsage() & Usage::MAP_WRITE) == Usage::MAP_WRITE) {
						updateOrWriteData = &writeData;
						rst = tr->map(0, 0, Usage::MAP_WRITE) == Usage::MAP_WRITE;
						printaln(L"Texture2DResource::map(w) : "sv, rst ? L"succeed"sv : L"failed"sv);
						if (rst) {
							rst = tr->write(0, 0, 0, writeData.data(), writeData.size()) == writeData.size();
							printaln(L"Texture2DResource::write : "sv, rst ? L"succeed"sv : L"failed"sv);
							tr->unmap(0, 0);
						}
					}

					auto tr2 = graphics->createTexture2DResource();
					auto rst2 = tr2->create(texDim, 0, 1, 1, texFmt, Usage::MAP_READ | Usage::COPY_DST, Usage::NONE);
					printaln(L"Texture2DResource::create(r) : "sv, rst2 ? L"succeed"sv : L"failed"sv);
					if (rst2) {
						rst = tr2->copyFrom(Vec3uz(), 0, 0, tr, 0, 0, Box3uz(Vec3uz(), Vec3uz(texDim[0], texDim[1], 1)));
						printaln(L"Texture2DResource::copyFrom : "sv, rst ? L"succeed"sv : L"failed"sv);

						if (rst) {
							std::vector<uint8_t> mappedData(mipBytes[0]);
							rst = tr2->map(0, 0, Usage::MAP_READ) == Usage::MAP_READ;
							printaln(L"Texture2DResource::map(r) : "sv, rst ? L"succeed"sv : L"failed"sv);
							if (rst) {
								auto readLen = tr2->read(0, 0, 0, mappedData.data(), mappedData.size());
								rst = readLen == mappedData.size();
								printaln(L"Texture2DResource::read : "sv, rst ? L"succeed"sv : L"failed"sv);
								tr2->unmap(0, 0);
							}

							auto isSame = true;
							if (updateOrWriteData) {
								for (size_t i = 0; i < updateOrWriteData->size(); ++i) {
									if ((*updateOrWriteData)[i] != mappedData[i]) {
										isSame = false;
										break;
									}
								}
							}

							if (isSame) {
								for (size_t i = updateOrWriteData ? updateOrWriteData->size() : 0; i < mipBytes[0]; ++i) {
									if (texData[i] != mappedData[i]) {
										isSame = false;
										break;
									}
								}
							}

							printaln(L"Texture2DResource texData check : "sv, isSame ? L"succeed"sv : L"failed"sv);
						}
					}
				}
			}

			auto sampler = graphics->createSampler();
			printaln(L"createSampler : "sv, sampler ? L"succeed"sv : L"failed"sv);
			if (sampler) {
				sampler->setAddress(SamplerAddressMode::REPEAT, SamplerAddressMode::REPEAT, SamplerAddressMode::REPEAT);
				sampler->setBorderColor(Vec4f32(0.1f, 0.2f, 0.4f, 0.5f));
				sampler->setComparisonFunc(ComparisonFunc::NEVER);
				sampler->setFilter(SamplerFilterOperation::COMPARISON, SamplerFilterMode::ANISOTROPIC, SamplerFilterMode::ANISOTROPIC, SamplerFilterMode::ANISOTROPIC);
				sampler->setMaxAnisotropy(2);
			}

			IntrusivePtr shader = new Shader();
			auto shaderResourcesFolder = Application::getAppPath().parent_path().u8string() + "/Resources/shaders/";
#ifdef SRK_HAS_SHADER_SCRIPT_H
			extensions::ShaderScript::set(shader, graphics, readFile(Application::getAppPath().parent_path().u8string() + "/Resources/shaders/lighting.shader"),
				[shaderResourcesFolder](const ProgramIncludeInfo& info) {
					return readFile(shaderResourcesFolder + info.file);
				},
				[](const ProgramInputInfo& info) {
					return ProgramInputDescriptor();
				}, programTranspileHandler);
#endif

			auto program = shader->select(nullptr);
			printaln(L"Program : "sv, program ? L"succeed"sv : L"failed"sv);
			if (program) {
				auto& info = program->getInfo();
				IntrusivePtr vac = new VertexAttributeCollection();

				auto pos = graphics->createVertexBuffer();
				float32_t posData[] = { 1.0f, 2.0f, 3.0f, 1.0f, 2.0f, 3.0f, 1.0f, 2.0f,
					1.0f, 2.0f, 3.0f, 1.0f, 2.0f, 3.0f, 1.0f, 2.0f,
					1.0f, 2.0f, 3.0f, 1.0f, 2.0f, 3.0f, 1.0f, 2.0f };
				pos->create(96, Usage::NONE, Usage::NONE, posData, 96);
				pos->setStride(32);
				vac->set(ShaderPredefine::POSITION0, VertexAttribute<IVertexBuffer>(pos, 3, VertexType::F32, 0));
				vac->set(ShaderPredefine::NORMAL0, VertexAttribute<IVertexBuffer>(pos, 3, VertexType::F32, 12));
				vac->set(ShaderPredefine::UV0, VertexAttribute<IVertexBuffer>(pos, 2, VertexType::F32, 24));

				IntrusivePtr spc = new ShaderParameterCollection();
				spc->set(ShaderPredefine::MATRIX_LW, new ShaderParameter())->set(Matrix3x4f32());
				spc->set(ShaderPredefine::MATRIX_LP, new ShaderParameter())->set(Matrix4x4f32());
				spc->set(ShaderPredefine::AMBIENT_COLOR, new ShaderParameter())->set(Vec3f32());
				spc->set(ShaderPredefine::DIFFUSE_COLOR, new ShaderParameter())->set(Vec3f32::ONE);
				spc->set(ShaderPredefine::SPECULAR_COLOR, new ShaderParameter())->set(Vec3f32::ONE);
				spc->set("_diffuseTexSampler", new ShaderParameter())->set(sampler);

				graphics->draw(program, vac, spc, nullptr);
			}
		}

		printaln(L"================================"sv);

		win->getEventDispatcher()->addEventListener(WindowEvent::RESIZED, createEventListener<WindowEvent>([graphics](Event<WindowEvent>& e) {
			graphics->setBackBufferSize(((IWindow*)e.getTarget())->getContentSize());
			}));

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSING, createEventListener<WindowEvent>([](Event<WindowEvent>& e) {
			//*e.getData<bool>() = true;
			}));

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
			looper->stop();
			}));

		looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([graphics](Event<LooperEvent>& e) {
			}));

		//(new Stats())->run(looper);
		win->setVisible(true);
		looper->run(true);

		return 0;
	}
};