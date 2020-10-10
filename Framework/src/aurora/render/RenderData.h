#pragma once

#include "aurora/math/Matrix.h"
#include "aurora/render/RenderPass.h"
#include "aurora/render/RenderPriority.h"

namespace aurora {
	class Material;
	class Mesh;
	class MeshResource;
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

		template<typename T>
		class Getter {
		public:
			using Fn = const T*(*)(void*);

			Getter(Fn fn = nullptr, void* target = nullptr) :
				_fn(fn),
				_target(target) {
			}

			Getter(const Getter& value) :
				_fn(value._fn),
				_target(value._target) {
			}

			Getter(Getter&& value) noexcept :
				_fn(value._fn),
				_target(value._target) {
			}

			inline AE_CALL operator bool() const {
				return _fn;
			}

			inline Getter& AE_CALL operator=(const Getter& value) {
				set(value._fn, value._target);
				return *this;
			}

			inline Getter& AE_CALL operator=(const std::nullptr_t) {
				set(nullptr, nullptr);
				return *this;
			}

			inline const T* AE_CALL operator()() {
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
		Getter<Mesh> mesh;
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
			mesh = data.mesh;
			renderer = data.renderer;
			subPasses = data.subPasses;
		}

		inline void AE_CALL reset() {
			renderable = nullptr;
			state = nullptr;
			priority.reset();
			material = nullptr;
			mesh = nullptr;
			renderer = nullptr;
			subPasses = nullptr;
		}
	};
}