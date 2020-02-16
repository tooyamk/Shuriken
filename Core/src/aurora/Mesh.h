#pragma once

#include "aurora/Ref.h"
#include "aurora/GraphicsBuffer.h"

namespace aurora {
	class AE_DLL Mesh : public Ref {
	public:
		Mesh();

		inline VertexBufferCollection& AE_CALL getVertexBuffers() {
			return _vertexBuffers;
		}

		inline const VertexBufferCollection& AE_CALL getVertexBuffers() const {
			return _vertexBuffers;
		}

		inline modules::graphics::IIndexBuffer* AE_CALL getIndexBuffer() const {
			return _indexBuffer;
		}

		inline void AE_CALL setIndexBuffer(modules::graphics::IIndexBuffer* buffer) {
			_indexBuffer = buffer;
		}

	protected:
		VertexBufferCollection _vertexBuffers;
		RefPtr<modules::graphics::IIndexBuffer> _indexBuffer;
	};
}