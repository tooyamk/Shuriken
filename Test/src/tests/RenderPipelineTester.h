#pragma once

#include "../BaseTester.h"
#include "aurora/SerializableObject.h"

class RenderPipelineTester : public BaseTester {
public:
	virtual int32_t AE_CALL run() override {
		auto monitors = Monitor::getMonitors();
		auto vms = monitors[0].getVideoModes();
		
		RefPtr app = new Application("TestApp");

		Application::Style wndStype;
		wndStype.thickFrame = true;
		if (app->createWindow(wndStype, "", Box2i32ui32(Vec2i32(100, 100), Vec2ui32(800, 600)), false)) {
			RefPtr gml = new GraphicsModuleLoader();

			//if (gml->load(getDLLName("ae-win-gl"))) {
			if (gml->load(getDLLName("ae-win-d3d11"))) {
				RefPtr gpstml = new ModuleLoader<IProgramSourceTranslator>();
				gpstml->load(getDLLName("ae-program-source-translator"));
				RefPtr gpst = gpstml->create(&Args().add("dxc", getDLLName("dxcompiler")));

				RefPtr graphics = gml->create(&Args().add("app", &*app).add("sampleCount", SampleCount(4)).add("trans", &*gpst));

				if (graphics) {
					println("Graphics Version : ", graphics->getVersion());

					graphics->getEventDispatcher().addEventListener(GraphicsEvent::ERR, createEventListener<GraphicsEvent>([](Event<GraphicsEvent>& e) {
						println(*(std::string_view*)e.getData());
						int a = 1;
					}));

					//graphics->getEventDispatcher().addEventListener(GraphicsEvent::ERR, new EventListener([](Event<GraphicsEvent>& e) {
					//	println(*(std::string_view*)e.getData());
					//	int a = 1;
					//}));

					struct {
						RefPtr<Application> app;
						RefPtr<Looper> looper;
						RefPtr<IGraphicsModule> g;
						RefPtr<Node> wrold;
						RefPtr<Node> model;
						RefPtr<Camera> camera;
						RefPtr<Material> material;
						RefPtr<Material> material2;
						RefPtr<StandardRenderPipeline> renderPipeline;
						std::vector<RefPtr<ILight>> lights;
						std::vector<RefPtr<IRenderable>> renderables;
					} renderData;
					renderData.app = app;
					renderData.looper = new Looper(1000.0 / 60.0);
					renderData.g = graphics;

					{
						RefPtr worldNode = new Node();
						RefPtr modelNode = worldNode->addChild<Node>();
						//modelNode = new Node();
						//modelNode->setLocalScale(Vec3f32(4));
						//modelNode->parentTranslate(Vec3f32(0, 3000.f, 0));
						//modelNode->localRotate(Quaternion::createFromEulerY(Math::PI<f32> * 0.8f));
						//modelNode->localRotate(Quaternion::createFromEulerY(Math::PI<f32>* 0.15f));
						//auto lm = modelNode->getLocalMatrix();
						//auto wm = modelNode->getWorldMatrix();
						renderData.model = modelNode;
						if (1) {
							RefPtr lightNode = worldNode->addChild<Node>();
							lightNode->setLocalPosition(Vec3f32(-100, 0, -100));
							auto light = renderData.lights.emplace_back(new PointLight());
							light->attachNode(lightNode);
							//light->setRadius(200);
							lightNode->localRotate(Quaternion::createFromEulerY(Math::PI_4<float32_t>));
							renderData.lights.emplace_back(light);
						}
						if (1) {
							RefPtr lightNode = worldNode->addChild<Node>();
							lightNode->setLocalPosition(Vec3f32(100, 0, -100));
							auto light = renderData.lights.emplace_back(new PointLight());
							light->attachNode(lightNode);
							//light->setRadius(1000);
							lightNode->localRotate(Quaternion::createFromEulerY(-Math::PI_4<float32_t>));
							renderData.lights.emplace_back(light);
						}
						RefPtr cameraNode = worldNode->addChild<Node>();
						//auto camera = new Camera();
						//cameraNode->addComponent(camera);
						renderData.camera = new Camera();
						renderData.camera->attachNode(cameraNode);
						auto mat = new Material();
						renderData.material = mat;
						auto mat2 = new Material();
						renderData.material2 = mat2;
						RefPtr renderer = new ForwardRenderer(*graphics);

						{
							auto size = app->getInnerSize();
							renderData.camera->setProjectionMatrix(Matrix44::createPerspectiveFovLH(Math::PI<float32_t> / 6.f, size[0] / size[1], 10, 10000));
							renderData.camera->getNode()->localTranslate(Vec3f32(0.f, 0.f, -200.f));
						}

						RenderTag forwardBaseTag("forward_base");
						RenderTag forwardAddTag("forward_add");

						RefPtr tag1 = new RenderTagCollection();
						tag1->add(forwardBaseTag);

						RefPtr tag2 = new RenderTagCollection();
						tag2->add(forwardAddTag);

						auto parsed = extensions::FBXConverter::parse(readFile(app->getAppPath().parent_path().u8string() + "/Resources/teapot.fbx"));
						for (auto& mr : parsed.meshes) {
							if (mr) {
								RefPtr rs = graphics->createRasterizerState();
								rs->setFillMode(FillMode::SOLID);
								rs->setFrontFace(FrontFace::CW);
								rs->setCullMode(CullMode::BACK);

								auto renderableMesh = new RenderableMesh();
								renderData.renderables.emplace_back(renderableMesh);
								renderableMesh->attachNode(modelNode);
								auto pass = new RenderPass();
								pass->state = new RenderState();
								pass->state->rasterizer.state = rs;
								pass->material = mat;
								pass->tags = tag1;
								renderableMesh->renderPasses.emplace_back(pass);
								{
									auto subPass = new RenderPass();
									subPass->state = new RenderState();
									subPass->state->rasterizer.state = rs;
									subPass->material = mat;
									subPass->tags = tag2;
									pass->subPasses.emplace_back(subPass);
								}
								auto mesh = new Mesh();
								renderableMesh->setMesh(mesh);
								renderableMesh->setRenderer(renderer);

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
							auto shaderResourcesFolder = app->getAppPath().parent_path().u8string() + "/Resources/shaders/";
							//s->upload(std::filesystem::path(app->getAppPath().parent_path().u8string() + "/Resources/shaders/test.shader"));
							extensions::ShaderScript::set(s, graphics, readFile(app->getAppPath().parent_path().u8string() + "/Resources/shaders/lighting.shader"),
								[shaderResourcesFolder](const Shader& shader, ProgramStage stage, const std::string_view& name) {
								return readFile(shaderResourcesFolder + name.data());
							});

							mat2->setShader(s);
							mat2->setParameters(new ShaderParameterCollection());
							{
								mat->getParameters()->set(ShaderPredefine::DIFFUSE_COLOR, new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec3f32::ONE);
								//mat->getParameters()->set(ShaderPredefine::DIFFUSE_COLOR, new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec3f32(1, 0, 0));
								//mat2->getParameters()->set(ShaderPredefine::DIFFUSE_COLOR, new ShaderParameter(ShaderParameterUsage::EXCLUSIVE))->set(Vec3f32(0, 1, 0));
							}
						}

						renderData.wrold = worldNode;
						renderData.renderPipeline = new StandardRenderPipeline();
						renderData.renderPipeline->getShaderParameters().set(ShaderPredefine::AMBIENT_COLOR, new ShaderParameter())->set(Vec3f32(0.0f));
						renderData.renderPipeline->getShaderParameters().set(ShaderPredefine::DIFFUSE_COLOR, new ShaderParameter())->set(Vec3f32::ONE);
						renderData.renderPipeline->getShaderParameters().set(ShaderPredefine::SPECULAR_COLOR, new ShaderParameter())->set(Vec3f32::ONE);
					}

					{
						auto texRes = graphics->createTexture2DResource();
						if (texRes) {
							auto img0 = extensions::PNGConverter::parse(readFile(app->getAppPath().parent_path().u8string() + "/Resources/white.png"));
							auto mipLevels = Image::calcMipLevels(img0->size);
							ByteArray mipsData0;
							std::vector<void*> mipsData0Ptr;
							img0->generateMips(img0->format, mipLevels, mipsData0, mipsData0Ptr);

							auto img1 = extensions::PNGConverter::parse(readFile(app->getAppPath().parent_path().u8string() + "/Resources/red.png"));
							ByteArray mipsData1;
							std::vector<void*> mipsData1Ptr;
							img1->generateMips(img1->format, mipLevels, mipsData1, mipsData1Ptr);

							mipsData0Ptr.insert(mipsData0Ptr.end(), mipsData1Ptr.begin(), mipsData1Ptr.end());

							auto hr = texRes->create(img0->size, 0, 1, 1, img0->format, Usage::IGNORE_UNSUPPORTED | Usage::MAP_WRITE, mipsData0Ptr.data());

							auto texView = graphics->createTextureView();
							texView->create(texRes, 0, -1, 0, -1);

							renderData.material->getParameters()->set(ShaderPredefine::DIFFUSE_TEXTURE, new ShaderParameter(ShaderParameterUsage::AUTO))->set(texView);
							renderData.material2->getParameters()->set(ShaderPredefine::DIFFUSE_TEXTURE, new ShaderParameter(ShaderParameterUsage::AUTO))->set(texView);
						}

						auto sam = graphics->createSampler();
						if (sam) {
							//sam->setMipLOD(0, 0);
							//sam->setAddress(SamplerAddressMode::WRAP, SamplerAddressMode::WRAP, SamplerAddressMode::WRAP);
							sam->setFilter(SamplerFilterOperation::NORMAL, SamplerFilterMode::POINT, SamplerFilterMode::POINT, SamplerFilterMode::POINT);
							renderData.material->getParameters()->set(ShaderPredefine::DIFFUSE_TEXTURE + "Sampler", new ShaderParameter(ShaderParameterUsage::AUTO))->set(sam);
							renderData.material2->getParameters()->set(ShaderPredefine::DIFFUSE_TEXTURE + "Sampler", new ShaderParameter(ShaderParameterUsage::AUTO))->set(sam);
						}
					}

					app->getEventDispatcher().addEventListener(ApplicationEvent::CLOSING, new EventListener(std::function([](Event<ApplicationEvent>& e) {
						//*e.getData<bool>() = true;
					})));

					app->getEventDispatcher().addEventListener(ApplicationEvent::CLOSED, new EventListener(std::function([renderData](Event<ApplicationEvent>& e) {
						renderData.looper->stop();
					})));

					renderData.looper->getEventDispatcher().addEventListener(LooperEvent::TICKING, new EventListener(std::function([renderData](Event<LooperEvent>& e) {
						auto dt = float32_t(*e.getData<int64_t>()) * 0.001f;

						renderData.app->pollEvents();

						renderData.model->localRotate(Quaternion::createFromEulerY(Math::PI<float32_t> * dt * 0.5f));

						renderData.g->setViewport(Box2i32ui32(Vec2i32::ZERO, renderData.app->getInnerSize()));
						renderData.renderPipeline->render(renderData.g, [renderData](render::IRenderCollector& collector) {
							collector.addCamera(renderData.camera);
							for (auto& r : renderData.renderables) collector.addRenderable(r);
							for (auto& l : renderData.lights) collector.addLight(l);
						});
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