#pragma once

#include "../BaseTester.h"

class RenderPipelineTester : public BaseTester {
public:
	virtual int32_t AE_CALL run() override {
		auto monitors = Monitor::getMonitors();
		auto vms = monitors[0].getVideoModes();

		RefPtr app = new Application(u8"TestApp");

		Application::Style wndStype;
		wndStype.thickFrame = true;
		if (app->createWindow(wndStype, u8"", Box2i32(Vec2i32({ 100, 100 }), Vec2i32({ 800, 600 })), false)) {
			RefPtr gml = new GraphicsModuleLoader();

			//if (gml->load(getDLLName("ae-win-gl"))) {
			if (gml->load(getDLLName("ae-win-d3d11"))) {
				RefPtr gpstml = new ModuleLoader<IProgramSourceTranslator>();
				gpstml->load(getDLLName("ae-program-source-translator"));
				RefPtr gpst = gpstml->create(&Args().add("dxc", getDLLName("dxcompiler")));

				RefPtr graphics = gml->create(&Args().add("app", &*app).add("sampleCount", SampleCount(4)).add("trans", &*gpst));

				if (graphics) {
					println("Graphics Version : ", graphics->getVersion());

					graphics->getEventDispatcher().addEventListener(GraphicsEvent::ERR, new EventListener(Recognitor<GraphicsEvent>(),[](Event<GraphicsEvent>& e) {
						println(*(std::string_view*)e.getData());
						int a = 1;
					}));

					struct {
						RefPtr<Application> app;
						RefPtr<Looper> looper;
						RefPtr<IGraphicsModule> g;
						RefPtr<Node> wrold;
						RefPtr<Node> model;
						RefPtr<Camera> camera;
						RefPtr<Material> material;
						RefPtr<StandardRenderPipeline> renderPipeline;
					} renderData;
					renderData.app = app;
					renderData.looper = new Looper(1000.0 / 60.0);
					renderData.g = graphics;

					{
						RefPtr worldNode = new Node();
						auto modelNode = worldNode->addChild(new Node());
						//modelNode = new Node();
						//modelNode->setLocalScale(Vec3f32(4));
						//modelNode->parentTranslate(Vec3f32(0, 3000.f, 0));
						//modelNode->localRotate(Quaternion::createFromEulerY(Math::PI<f32> * 0.8f));
						//modelNode->localRotate(Quaternion::createFromEulerY(Math::PI<f32>* 0.15f));
						auto lm = modelNode->getLocalMatrix();
						auto wm = modelNode->getWorldMatrix();
						renderData.model = modelNode;
						auto cameraNode = worldNode->addChild(new Node());
						auto camera = new Camera();
						renderData.camera = camera;
						cameraNode->addComponent(camera);
						auto mat = new Material();
						renderData.material = mat;
						RefPtr renderer = new ForwardRenderer(*graphics);

						{
							auto size = app->getInnerSize();
							camera->setProjectionMatrix(Matrix44::createPerspectiveFovLH(Math::PI<f32> / 6.f, size[0] / size[1], 10, 10000));
							camera->getNode()->localTranslate(Vec3f32(0.f, 0.f, -200.f));
						}

						auto parsed = extensions::FBXConverter::parse(readFile(app->getAppPath() + u8"Resources/teapot.fbx"));
						for (auto& mr : parsed.meshes) {
							if (mr) {
								RefPtr rs = graphics->createRasterizerState();
								rs->setFillMode(FillMode::SOLID);
								rs->setFrontFace(FrontFace::CW);
								rs->setCullMode(CullMode::NONE);

								auto renderableMesh = new RenderableMesh();
								auto pass = new RenderPass();
								pass->state = new RenderState();
								pass->state->rasterizer.state = rs;
								renderData.material = mat;
								pass->material = mat;
								renderableMesh->renderPasses.emplace_back(pass);
								auto mesh = new Mesh();
								renderableMesh->setMesh(mesh);
								renderableMesh->setRenderer(renderer);
								modelNode->addComponent(renderableMesh);

								for (auto& itr : mr->getVertexResources()) {
									auto vs = itr.second;
									auto vb = graphics->createVertexBuffer();
									vb->create(vs->data.getLength(), Usage::NONE, vs->data.getSource(), vs->data.getLength());
									vb->setFormat(vs->format);
									mesh->getVertexBuffers().set(itr.first, vb);
								}

								if (auto is = mr->indexResource; is) {
									auto ib = graphics->createIndexBuffer();
									ib->create(is->data.getLength(), Usage::NONE, is->data.getSource(), is->data.getLength());
									ib->setFormat(is->type);
									mesh->setIndexBuffer(ib);
								}
							}
						}

						{
							RefPtr s = new Shader();

							mat->setShader(s);
							mat->setParameters(new ShaderParameterCollection());
							std::string shaderResourcesFolder = app->getAppPath() + u8"Resources/shaders/";
							//s->upload(std::filesystem::path(app->getAppPath() + u8"Resources/shaders/test.shader"));
							extensions::ShaderScript::set(s, graphics, readFile(app->getAppPath() + u8"Resources/shaders/test.shader"),
								[shaderResourcesFolder](const Shader& shader, ProgramStage stage, const std::string_view& name) {
								return readFile(shaderResourcesFolder + name.data());
							});
							/*
							s->upload(graphics,
								new ProgramSource(readProgramSource(shaderResourcesFolder + "modelVert.hlsl", ProgramStage::VS)),
								new ProgramSource(readProgramSource(shaderResourcesFolder + "modelFrag.hlsl", ProgramStage::PS)),
								nullptr, 0, nullptr, 0,
								[&shaderResourcesFolder](const Shader& shader, ProgramStage stage, const std::string_view& name) {
								return readFile(shaderResourcesFolder + name.data());
							});
							*/

							{
								mat->getParameters()->set("red", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
								mat->getParameters()->set("green", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
								//cf->add("blue", new ShaderParameter())->set(Vec4f32::ONE).setUpdated();

								auto aabbccStruct = new ShaderParameterCollection();
								aabbccStruct->set("val1", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec4f32::ONE).setUpdated();
								f32 val2[] = { 1.0f, 1.0f };
								aabbccStruct->set("val2", new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set<f32>(val2, sizeof(val2), sizeof(f32), true).setUpdated();
								aabbccStruct->set("val3", new ShaderParameter())->set(Vec4f32::ONE).setUpdated();
								mat->getParameters()->set("blue", new ShaderParameter())->set(aabbccStruct);
							}
						}

						renderData.wrold = worldNode;
						renderData.renderPipeline = new StandardRenderPipeline();
					}

					{
						auto texRes = graphics->createTexture2DResource();
						if (texRes) {
							auto img0 = extensions::PNGConverter::parse(readFile(app->getAppPath() + u8"Resources/c4.png"));
							auto mipLevels = Image::calcMipLevels(img0->size);
							ByteArray mipsData0;
							std::vector<void*> mipsData0Ptr;
							img0->generateMips(img0->format, mipLevels, mipsData0, mipsData0Ptr);

							auto img1 = extensions::PNGConverter::parse(readFile(app->getAppPath() + u8"Resources/red.png"));
							ByteArray mipsData1;
							std::vector<void*> mipsData1Ptr;
							img1->generateMips(img1->format, mipLevels, mipsData1, mipsData1Ptr);

							mipsData0Ptr.insert(mipsData0Ptr.end(), mipsData1Ptr.begin(), mipsData1Ptr.end());

							auto hr = texRes->create(img0->size, 0, 1, 1, img0->format, Usage::IGNORE_UNSUPPORTED | Usage::MAP_WRITE, mipsData0Ptr.data());

							auto texView = graphics->createTextureView();
							texView->create(texRes, 0, -1, 0, -1);

							auto pb = graphics->createPixelBuffer();
							if (pb) {
								//pb->create(img0->size.getMultiplies() * 4, Usage::MAP_WRITE | Usage::PERSISTENT_MAP, mipsData0Ptr.data()[0], img0->size.getMultiplies() * 4);
								//texRes->copyFrom(0, 0, Box2ui32(Vec2ui32(0, 0), Vec2ui32(4, 4)), pb);
							}

							uint8_t texData[] = { 255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 0, 255 };
							//auto mapped = texRes->map(0, 0, Usage::MAP_WRITE);
							//texRes->write(0, 0, 4, texData, sizeof(texData));
							//texRes->unmap(0, 0);
							//texRes->update(0, 0, Box2ui32(Vec2ui32(1, 1), Vec2ui32(2, 2)), texData);

							renderData.material->getParameters()->set("texDiffuse", new ShaderParameter(ShaderParameterUsage::AUTO))->set(texView).setUpdated();
						}

						auto sam = graphics->createSampler();
						if (sam) {
							//sam->setMipLOD(0, 0);
							//sam->setAddress(SamplerAddressMode::WRAP, SamplerAddressMode::WRAP, SamplerAddressMode::WRAP);
							sam->setFilter(SamplerFilterOperation::NORMAL, SamplerFilterMode::POINT, SamplerFilterMode::POINT, SamplerFilterMode::POINT);
							renderData.material->getParameters()->set("samLiner", new ShaderParameter(ShaderParameterUsage::AUTO))->set(sam).setUpdated();
						}
					}

					app->getEventDispatcher().addEventListener(ApplicationEvent::CLOSING, new EventListener(std::function([](Event<ApplicationEvent>& e) {
						//*e.getData<bool>() = true;
					})));

					app->getEventDispatcher().addEventListener(ApplicationEvent::CLOSED, new EventListener(std::function([renderData](Event<ApplicationEvent>& e) {
						renderData.looper->stop();
					})));

					renderData.looper->getEventDispatcher().addEventListener(LooperEvent::TICKING, new EventListener(std::function([renderData](Event<LooperEvent>& e) {
						auto dt = f32(*e.getData<int64_t>()) * 0.001f;

						renderData.app->pollEvents();

						renderData.model->localRotate(Quaternion::createFromEulerY(Math::PI<f32> * dt * 0.5f));

						//renderData.g->beginRender();
						//renderData.g->clear(ClearFlag::COLOR | ClearFlag::DEPTH | ClearFlag::STENCIL, Vec4f32(0.0f, 0.0f, 0.25f, 1.0f), 1.0f, 0);

						renderData.renderPipeline->render(renderData.g, renderData.camera, renderData.wrold);

						//renderData.g->endRender();
						renderData.g->present();
					})));

					(new Stats())->run(renderData.looper);
					app->setVisible(true);
					renderData.looper->run(true);
				}
			}
		}

		return 0;
	}

private:
	RefPtr<Node> _worldRoot;
};