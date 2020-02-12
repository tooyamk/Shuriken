#pragma once

#include "aurora/Ref.h"
#include "aurora/RTTI.h"

namespace aurora {
	class Node;
}

namespace aurora::components {
	class AE_DLL IComponent : public Ref {
	public:
		virtual ~IComponent() {}

		uint32_t layer;

		inline bool AE_CALL isEnalbed() const {
			return _enabled;
		}
		void AE_CALL setEnabled(bool b);
		inline Node* AE_CALL getNode() const {
			return _node;
		}

		AE_RTTI_GEN_IS_KIND_OF_METHOD();

	protected:
		bool _enabled;
		Node* _node;

		friend Node;

		IComponent();

		void AE_CALL __setNode(Node* node);

		virtual void AE_CALL _nodeChanged(Node* old) {};
		virtual void AE_CALL _enabledChanged() {};

		AE_RTTI_DECLARE_BASE();
	};
}
