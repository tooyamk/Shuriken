#pragma once

#include "base/Ref.h"

namespace aurora::nodes {
	class Node;
}

namespace aurora::nodes::component {
	class AE_DLL ComponentFlag {
	public:
		AE_DECLA_CANNOT_INSTANTIATE(ComponentFlag);

		static const uint32_t CAMERA = 0b1;
		static const uint32_t RENDERABLE = 0b10;
	};


	class AE_DLL AbstractComponent : public Ref {
	public:
		uint32_t flag;
		uint32_t layer;

		inline bool AE_CALL getEnalbed() const;
		void AE_CALL setEnabled(bool b);
		inline Node* AE_CALL getNode() const;

	ae_internal_public:
		void AE_CALL __setNode(Node* node);

	protected:
		bool _enabled;
		Node* _node;

		AbstractComponent();

		virtual void AE_CALL _nodeChanged(Node* old) {};
		virtual void AE_CALL _enabledChanged() {};
	};
}

#include "AbstractComponent.inl"
