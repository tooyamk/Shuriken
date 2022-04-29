#pragma once

#include "aurora/Core.h"

#include "aurora/Application.h"
#include "aurora/GraphicsBuffer.h"
#include "aurora/Image.h"
#include "aurora/Looper.h"
#include "aurora/Material.h"
#include "aurora/Mesh.h"
#include "aurora/Monitor.h"
#include "aurora/Node.h"
#include "aurora/ProgramSource.h"
#include "aurora/Shader.h"
#include "aurora/ShaderDefine.h"
#include "aurora/ShaderParameter.h"
#include "aurora/ShaderPredefine.h"

#include "aurora/modules/ModuleLoader.h"
#include "aurora/modules/graphics/GraphicsAdapter.h"
#include "aurora/modules/graphics/IShaderTranspiler.h"

#include "aurora/components/Camera.h"
#include "aurora/components/renderables/RenderableMesh.h"
#include "aurora/components/lights/DirectionLight.h"
#include "aurora/components/lights/PointLight.h"
#include "aurora/components/lights/SpotLight.h"

#include "aurora/render/ForwardRenderer.h"
#include "aurora/render/SpriteRenderer.h"
#include "aurora/render/StandardRenderPipeline.h"