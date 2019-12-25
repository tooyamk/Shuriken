#pragma once

#include "IGraphicsModule.h"

namespace aurora::modules::graphics {
	template<typename T>
	class AE_TEMPLATE_DLL MultipleBuffer : public IObject {
	public:
		struct Node {
			RefPtr<T> target;
			Node* next = nullptr;
		};


		MultipleBuffer(IGraphicsModule& graphics, uint8_t max) : IObject(graphics),
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

		bool AE_CALL create(uint32_t size, Usage bufferUsage, const void* data, uint32_t dataSize) {
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

		inline uint32_t AE_CALL getSize() const {
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

		inline uint32_t AE_CALL read(uint32_t offset, void* dst, uint32_t dstLen) {
			return _cur ? _cur->target.get()->read(offset, dst, dstLen) : -1;
		}

		inline uint32_t AE_CALL write(uint32_t offset, const void* data, uint32_t length) {
			return _cur ? _cur->target.get()->write(offset, data, length) : -1;
		}

		inline uint32_t AE_CALL update(uint32_t offset, const void* data, uint32_t length) {
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

		inline uint8_t getCount() const {
			return _count;
		}

	private:
		uint8_t _count;
		uint8_t _max;

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
		MultipleVertexBuffer(IGraphicsModule& graphics, uint8_t max);
		virtual ~MultipleVertexBuffer();

		virtual const void* AE_CALL getNativeBuffer() const override;
		virtual bool AE_CALL create(uint32_t size, Usage bufferUsage, const void* data = nullptr, uint32_t dataSize = 0) override;
		virtual uint32_t AE_CALL getSize() const override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual uint32_t AE_CALL read(uint32_t offset, void* dst, uint32_t dstLen) override;
		virtual uint32_t AE_CALL write(uint32_t offset, const void* data, uint32_t length) override;
		virtual uint32_t AE_CALL update(uint32_t offset, const void* data, uint32_t length) override;
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
		MultipleIndexBuffer(IGraphicsModule& graphics, uint8_t max);
		virtual ~MultipleIndexBuffer();

		virtual const void* AE_CALL getNativeBuffer() const override;
		virtual bool AE_CALL create(uint32_t size, Usage bufferUsage, const void* data = nullptr, uint32_t dataSize = 0) override;
		virtual uint32_t AE_CALL getSize() const override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual uint32_t AE_CALL read(uint32_t offset, void* dst, uint32_t dstLen) override;
		virtual uint32_t AE_CALL write(uint32_t offset, const void* data, uint32_t length) override;
		virtual uint32_t AE_CALL update(uint32_t offset, const void* data, uint32_t length) override;
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
		MultipleConstantBuffer(IGraphicsModule& graphics, uint8_t max);
		virtual ~MultipleConstantBuffer();

		virtual const void* AE_CALL getNativeBuffer() const override;
		virtual bool AE_CALL create(uint32_t size, Usage bufferUsage, const void* data = nullptr, uint32_t dataSize = 0) override;
		virtual uint32_t AE_CALL getSize() const override;
		virtual Usage AE_CALL getUsage() const override;
		virtual Usage AE_CALL map(Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual uint32_t AE_CALL read(uint32_t offset, void* dst, uint32_t dstLen) override;
		virtual uint32_t AE_CALL write(uint32_t offset, const void* data, uint32_t length) override;
		virtual uint32_t AE_CALL update(uint32_t offset, const void* data, uint32_t length) override;
		//virtual void AE_CALL flush() override;
		virtual bool AE_CALL isSyncing() const override;

	private:
		MultipleBuffer<IConstantBuffer> _base;
	};
}