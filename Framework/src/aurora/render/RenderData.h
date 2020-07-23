#pragma once

#include "aurora/math/Matrix.h"
#include "aurora/render/RenderPass.h"
#include "aurora/render/RenderPriority.h"

namespace aurora {
	class Material;
	class Mesh;
}

namespace aurora::components::renderables {
	class IRenderable;
}

namespace aurora::render {
	class IRenderer;
	class RenderState;


	class AE_FW_DLL RenderData {
	public:
		RenderData() {
			reset();
		}

		class MeshGetter {
		public:
			using Fn = const Mesh* (*)(void*);

			MeshGetter(Fn fn = nullptr, void* target = nullptr) :
				_fn(fn),
				_target(target) {
			}

			inline AE_CALL operator bool() const {
				return _fn;
			}

			inline MeshGetter& AE_CALL operator=(const MeshGetter& value) {
				set(value._fn, value._target);
				return *this;
			}

			inline MeshGetter& AE_CALL operator=(const nullptr_t) {
				set(nullptr, nullptr);
				return *this;
			}

			inline const Mesh* AE_CALL operator()() {
				return _fn(_target);
			}

			inline void AE_CALL set(Fn fn, void* target) {
				this->_fn = fn;
				this->_target = target;
			}

			inline void AE_CALL reset() {
				set(nullptr, nullptr);
			}

		private:
			Fn _fn;
			void* _target;
		};

		const components::renderables::IRenderable* renderable;
		RenderPriority priority;
		RenderState* state;
		Material* material;
		MeshGetter meshGetter;
		IRenderer* renderer;
		std::vector<RefPtr<RenderPass>>* subPasses;

		struct {
			Matrix34 l2w;
			Matrix34 l2v;
			Matrix44 l2p;
		} matrix;

		inline void AE_CALL set(const RenderData& data) {
			renderable = data.renderable;
			state = data.state;
			priority = data.priority;
			material = data.material;
			meshGetter = data.meshGetter;
			renderer = data.renderer;
			subPasses = data.subPasses;
		}

		inline void AE_CALL reset() {
			renderable = nullptr;
			state = nullptr;
			priority.reset();
			material = nullptr;
			meshGetter = nullptr;
			renderer = nullptr;
			subPasses = nullptr;
		}
	};
}