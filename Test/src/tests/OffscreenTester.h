﻿#pragma once

#include "../BaseTester.h"

class OffscreenTester : public BaseTester {
public:
	virtual int32_t SRK_CALL run() override {
		auto monitors = Monitor::getMonitors();
		auto vms = monitors[0].getVideoModes();

		IntrusivePtr gml = new GraphicsModuleLoader();

		//if (!gml->load(getDLLName("srk-module-graphics-gl"))) return 0;
		if (!gml->load(getDllPath(u8"srk-module-graphics-d3d11"))) return 0;

		CreateGrahpicsModuleDescriptor createGrahpicsModuleDesc;
		createGrahpicsModuleDesc.sampleCount = 1;
		createGrahpicsModuleDesc.offscreen = true;
		createGrahpicsModuleDesc.driverType = DriverType::SOFTWARE;
		createGrahpicsModuleDesc.debug = Environment::IS_DEBUG;
		createGrahpicsModuleDesc.createProcessInfoHandler = [](const std::string_view& msg) {
			printaln(msg);
		};

		auto graphics = gml->create(createGrahpicsModuleDesc);
		if (!graphics) return 0;

		printaln(L"Graphics Version : "sv, graphics->getVersion());

		graphics->getEventDispatcher()->addEventListener(GraphicsEvent::ERR, createEventListener<GraphicsEvent>([](Event<GraphicsEvent>& e) {
			printaln(*(std::string_view*)e.getData());
			int a = 1;
			}));

		{
			{
				auto rs = graphics->createRasterizerState();
				rs->setFillMode(FillMode::SOLID);
				rs->setFrontFace(FrontFace::CW);
				rs->setCullMode(CullMode::NONE);
				graphics->setRasterizerState(rs);
			}

			auto tr = graphics->createTexture2DResource();
			auto rv = graphics->createRenderView();
			auto rt = graphics->createRenderTarget();
			tr->create(Vec2uz(800, 600), 0, 1, 1, TextureFormat::R8G8B8A8_UNORM, Usage::RENDERABLE, Usage::NONE);
			rv->create(tr, 0, 0, 0);
			rt->setRenderView(0, rv);

			auto program = graphics->createProgram();
			{
				const std::string _vs = R"(
struct VS_INPUT {
    float2 position : POSITION0;
    float2 uv : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input) {
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.position = float4(input.position.x, input.position.y, 0.5f, 1.0f);
    output.uv = float2(input.uv.x, 1.0f - input.uv.y);
    return output;
}
)";
				const std::string _ps = R"(
struct PS_INPUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET {
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}
)";

				ProgramSource vs;
				vs.language = ProgramLanguage::HLSL;
				vs.stage = ProgramStage::VS;
				vs.data = ByteArray((void*)_vs.data(), _vs.size(), ByteArray::Usage::SHARED);
				ProgramSource ps;
				ps.language = ProgramLanguage::HLSL;
				ps.stage = ProgramStage::VS;
				ps.data = ByteArray((void*)_ps.data(), _ps.size(), ByteArray::Usage::SHARED);
				program->create(vs, ps, nullptr, 0, nullptr, nullptr, programTranspileHandler);
			}

			IntrusivePtr vertexBuffers = new VertexAttributeCollection();
			{
				float32_t vd[] = {
						-1.f, 1.f,
						1.f, 1.f,
						1.f, -1.f,
						-1.f, -1.f };

				using type = std::remove_cvref_t<decltype(vd[0])>;
				constexpr auto size = std::extent_v<decltype(vd)> *sizeof(type);

				auto vb = graphics->createVertexBuffer();
				vb->create(size, Usage::NONE, Usage::NONE, vd, size);
				vb->setStride(2 * sizeof(type));
				vertexBuffers->set("POSITION0", VertexAttribute<IVertexBuffer>(vb, 2, VertexType::F32, 0));

				type uvd[] = {
						0.f, 0.f,
						1.f, 0.f,
						1.f, 1.f,
						0.f, 1.f };
				auto uvb = graphics->createVertexBuffer();
				uvb->create(size, Usage::NONE, Usage::NONE, uvd, size);
				uvb->setStride(2 * sizeof(type));
				vertexBuffers->set("TEXCOORD0", VertexAttribute<IVertexBuffer>(uvb, 2, VertexType::F32, 0));
			}

			IntrusivePtr shaderParameters = new ShaderParameterCollection();
			{
				//shaderParameters->set("tex", new ShaderParameter(ShaderParameterUsage::AUTO))->set(tv, ShaderParameterUpdateBehavior::FORCE);
				//shaderParameters->set("texSampler", new ShaderParameter(ShaderParameterUsage::AUTO))->set(_pointTexSampler, ShaderParameterUpdateBehavior::FORCE);
			}

			auto indices = graphics->createIndexBuffer();
			{
				uint16_t id[] = {
						0, 1, 2,
						0, 2, 3 };

				using type = std::remove_cvref_t<decltype(id[0])>;
				constexpr auto size = std::extent_v<decltype(id)> *sizeof(type);

				indices->create(size, Usage::NONE, Usage::NONE, id, size);
				indices->setFormat<type>();
			}

			graphics->setRenderTarget(rt);
			graphics->setViewport(Box2i32ui32(Vec2i32::ZERO, tr->getDimensions().cast<2>()));
			graphics->beginRender();
			graphics->clear(ClearFlag::COLOR | ClearFlag::DEPTH | ClearFlag::STENCIL, Vec4f32(0.0f, 0.0f, 0.25f, 1.0f), 1.0f, 0);
			graphics->setBlendState(nullptr);
			graphics->setDepthStencilState(nullptr);
			graphics->draw(program, vertexBuffers, shaderParameters, indices);
			graphics->setRenderTarget(nullptr);

			{
				auto tex = graphics->createTexture2DResource();
				if (tex->create(tr->getDimensions(), 0, 1, 1, tr->getFormat(), Usage::MAP_READ, Usage::NONE)) {
					if (tex->copyFrom(Vec3ui32::ZERO, 0, 0, tr, 0, 0, Box3ui32(Vec3ui32::ZERO, tr->getDimensions()))) {
						if (tex->map(0, 0, Usage::MAP_READ) != modules::graphics::Usage::NONE) {
							auto pixelsSize = tr->getDimensions().getMultiplies() * 4;
							ByteArray pixels(pixelsSize, pixelsSize);
							tex->read(0, 0, 0, pixels.getSource(), pixels.getLength());
							tex->unmap(0, 0);

							Image img;
							img.format = modules::graphics::TextureFormat::R8G8B8A8_UNORM;
							img.dimensions = tr->getDimensions().cast<2>();
							img.source = std::move(pixels);

							///*
#ifdef SRK_HAS_PNG_CONVERTER_H
							auto aaa = extensions::PNGConverter::decode(readFile("D:/Users/Sephiroth/Desktop/wall.png"));
							if (aaa && aaa->format == modules::graphics::TextureFormat::R8G8B8_UNORM) {
								ByteArray dst(aaa->dimensions.getMultiplies() * 4);
								dst.setLength(dst.getCapacity());
								Image::convertFormat(aaa->dimensions, aaa->format, aaa->source.getSource(), modules::graphics::TextureFormat::R8G8B8A8_UNORM, dst.getSource());
								aaa->format = modules::graphics::TextureFormat::R8G8B8A8_UNORM;
								aaa->source = std::move(dst);
							}
							if (aaa) aaa->flipY();
							//*/

							///*
#	ifdef SRK_HAS_ASTC_CONVERTER_H
							auto t0 = srk::Time::now();
							auto out = extensions::ASTCConverter::encode(*aaa, Vec3ui32(4, 4, 1), extensions::ASTCConverter::Profile::LDR, extensions::ASTCConverter::Quality::FASTEST, extensions::ASTCConverter::Flags::NONE, 10);
							if (out.isValid()) {
								printaln(L"use time : "sv, srk::Time::now() - t0);
								writeFile("D:/Users/Sephiroth/Desktop/6x6.astc", out);
								printaln(L"doneeeeeeeeeeeeeeeeee"sv);
							}
#	endif
#endif
							//*/

							//writeFile(Application::getAppPath().parent_path().string() + "/offscreen.png", extensions::PNGConverter::encode(img));
						}
					}
				}
			}
		}

		IntrusivePtr looper = new Looper(1000.0 / 60.0);

		looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, new EventListener(std::function([](Event<LooperEvent>& e) {
			auto dt = float64_t(*e.getData<int64_t>());
			})));

		(new Stats())->run(looper);
		//app->setVisible(true);
		looper->run(true);

		return 0;
	}
};