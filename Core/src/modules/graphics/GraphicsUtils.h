#pragma once

#include "IGraphicsModule.h"

namespace aurora::modules::graphics {
	template<typename T>
	class AE_TEMPLATE_DLL MultipleBuffer : public IObject {
	public:
		struct Node {
			RefPtr<T> target;
			Node* next;
		};


		MultipleBuffer(IGraphicsModule& graphics, ui8 max) : IObject(graphics),
			_count(0),
			_max(max ? max : 1),
			_head(nullptr),
			_cur(nullptr) {
		}

		virtual ~MultipleBuffer() {
			_release();
		}

		inline const void* AE_CALL getNativeBuffer() const {
			return _cur ? _cur->target.get()->getNativeBuffer() : nullptr;
		}

		bool AE_CALL create(ui32 size, Usage bufferUsage, const void* data, ui32 dataSize) {
			_release();

			if (auto buf = _createBuffer(); buf) {
				_head = new Node();
				_head->target = buf;
				_head->next = _head;
				_count = 1;

				_cur = _head;

				buf->create(size, bufferUsage, data, dataSize);

				return true;
			}

			return false;
		}

		inline ui32 AE_CALL getSize() const {
			return _cur ? _cur->target.get()->getSize() : 0;
		}

		inline Usage AE_CALL getUsage() const {
			return _cur ? _cur->target.get()->getUsage() : Usage::NONE;
		}

		Usage AE_CALL map(Usage expectMapUsage) {
			if (_cur && expectMapUsage != Usage::NONE) {
				if (auto buf = _cur->target.get(); _max > 1 && (expectMapUsage & Usage::MAP_SWAP) == Usage::MAP_SWAP) {
					if (auto forceSwap = (expectMapUsage & Usage::MAP_FORCE_SWAP) == Usage::MAP_FORCE_SWAP; forceSwap || buf->isSyncing()) {
						auto needCreate = true;
						if (forceSwap) {
							if (_count >= _max) needCreate = false;
						} else if (_count > 1) {
							if (_cur->next->target.get()->isSyncing()) {
								if (_count >= _max) needCreate = false;
							} else {
								needCreate = false;
							}
						}

						if (needCreate) {
							if (auto newBuf = _createBuffer(); newBuf) {
								auto node = new Node();
								node->target = newBuf;
								node->next = _cur->next;
								_cur->next = node;
								_cur = node;
								++_count;

								newBuf->create(buf->getSize(), buf->getUsage());

								if constexpr (std::is_base_of_v<IVertexBuffer, T>) {
									VertexSize size;
									VertexType type;
									buf->getFormat(&size, &type);
									newBuf->setFormat(size, type);
								}

								auto usage = newBuf->map(expectMapUsage);
								if (usage != Usage::NONE) usage |= Usage::DISCARD;
								return usage;
							} else {
								_cur = _cur->next;
								return _cur->target.get()->map(expectMapUsage);
							}
						} else {
							_cur = _cur->next;
							return _cur->target.get()->map(expectMapUsage);
						}
					} else {
						return buf->map(expectMapUsage);
					}
				} else {
					return buf->map(expectMapUsage);
				}
			}

			return Usage::NONE;
		}

		inline void AE_CALL unmap() {
			if (_cur) _cur->target.get()->unmap();
		}

		inline ui32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen) {
			return _cur ? _cur->target.get()->read(offset, dst, dstLen) : -1;
		}

		inline ui32 AE_CALL write(ui32 offset, const void* data, ui32 length) {
			return _cur ? _cur->target.get()->write(offset, data, length) : -1;
		}

		inline ui32 AE_CALL update(ui32 offset, const void* data, ui32 length) {
			return _cur ? _cur->target.get()->update(offset, data, length) : -1;
		}

		inline bool AE_CALL isSyncing() const {
			return _cur ? _cur->target.get()->isSyncing() : false;
		}

		inline Node* getBegin() const {
			return _head;
		}

		inline Node* getCurrent() const {
			return _cur;
		}

		inline ui8 getCount() const {
			return _count;
		}

	private:
		ui8 _count;
		ui8 _max;

		Node* _head;
		Node* _cur;

		void AE_CALL _release() {
			if (_cur) {
				auto node = _head;
				auto n = _count;
				do {
					auto next = node->next;
					delete node;
					node = next;
				} while (--n > 0);

				_head = nullptr;
				_cur = nullptr;
				_count = 0;
			}
		}

		T* AE_CALL _createBuffer() {
			if constexpr (std::is_base_of_v<IVertexBuffer, T>) {
				return _graphics.get()->createVertexBuffer();
			} else if constexpr (std::is_base_of_v<IIndexBuffer, T>) {
				return _graphics.get()->createIndexBuffer();
			} else {
				return nullptr;
			}
		}
	};


	class AE_DLL MultipleVertexBuffer : public IVertexBuffer {
	public:
		MultipleVertexBuffer(IGraphicsModule& graphics, ui8 max);
		virtual ~MultipleVertexBuffer();

		virtual const void* AE_CALL getNativeBuffer() const override;
		virtual bool AE_CALL create(ui32 size, Usage bufferUsage, const void* data = nullptr, ui32 dataSize = 0) override;
		virtual ui32 AE_CALL getSize() const override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual ui32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen) override;
		virtual ui32 AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual ui32 AE_CALL update(ui32 offset, const void* data, ui32 length) override;
		virtual void AE_CALL getFormat(VertexSize* size, VertexType* type) const override;
		virtual void AE_CALL setFormat(VertexSize size, VertexType type) override;
		//virtual void AE_CALL flush() override;
		virtual bool AE_CALL isSyncing() const override;

	private:
		VertexSize _vertSize;
		VertexType _vertType;

		MultipleBuffer<IVertexBuffer> _base;
	};


	class AE_DLL MultipleIndexBuffer : public IIndexBuffer {
	public:
		MultipleIndexBuffer(IGraphicsModule& graphics, ui8 max);
		virtual ~MultipleIndexBuffer();

		virtual const void* AE_CALL getNativeBuffer() const override;
		virtual bool AE_CALL create(ui32 size, Usage bufferUsage, const void* data = nullptr, ui32 dataSize = 0) override;
		virtual ui32 AE_CALL getSize() const override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual ui32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen) override;
		virtual ui32 AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual ui32 AE_CALL update(ui32 offset, const void* data, ui32 length) override;
		virtual IndexType AE_CALL getFormat() const override;
		virtual void AE_CALL setFormat(IndexType type) override;
		//virtual void AE_CALL flush() override;
		virtual bool AE_CALL isSyncing() const override;

	private:
		IndexType _idxType;

		MultipleBuffer<IIndexBuffer> _base;
	};


	class AE_DLL MultipleConstantBuffer : public IConstantBuffer {
	public:
		MultipleConstantBuffer(IGraphicsModule& graphics, ui8 max);
		virtual ~MultipleConstantBuffer();

		virtual const void* AE_CALL getNativeBuffer() const override;
		virtual bool AE_CALL create(ui32 size, Usage bufferUsage, const void* data = nullptr, ui32 dataSize = 0) override;
		virtual ui32 AE_CALL getSize() const override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual ui32 AE_CALL read(ui32 offset, void* dst, ui32 dstLen) override;
		virtual ui32 AE_CALL write(ui32 offset, const void* data, ui32 length) override;
		virtual ui32 AE_CALL update(ui32 offset, const void* data, ui32 length) override;
		//virtual void AE_CALL flush() override;
		virtual bool AE_CALL isSyncing() const override;

	private:
		MultipleBuffer<IConstantBuffer> _base;
	};
}