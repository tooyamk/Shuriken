#pragma once

#include "aurora/modules/graphics/IGraphicsModule.h"
#include <unordered_map>

namespace aurora {
	template<typename T>
	class MultipleBuffer : public modules::graphics::IObject {
	public:
		struct Node {
			RefPtr<T> target;
			Node* next = nullptr;
		};


		MultipleBuffer(modules::graphics::IGraphicsModule& graphics, uint8_t max) : modules::graphics::IObject(graphics),
			_count(0),
			_max(max ? max : 1),
			_head(nullptr),
			_cur(nullptr) {
		}

		virtual ~MultipleBuffer() {
			destroy();
		}

		inline bool AE_CALL isCreated() const {
			return _cur ? _cur->target->isCreated() : false;
		}

		inline const void* AE_CALL getNative() const {
			return _cur ? _cur->target->getNative() : nullptr;
		}

		bool AE_CALL create(uint32_t size, modules::graphics::Usage bufferUsage, const void* data, uint32_t dataSize) {
			destroy();

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
			return _cur ? _cur->target->getSize() : 0;
		}

		inline modules::graphics::Usage AE_CALL getUsage() const {
			return _cur ? _cur->target->getUsage() : modules::graphics::Usage::NONE;
		}

		modules::graphics::Usage AE_CALL map(modules::graphics::Usage expectMapUsage) {
			if (_cur && expectMapUsage != modules::graphics::Usage::NONE) {
				if (auto buf = _cur->target.get(); _max > 1 && (expectMapUsage & modules::graphics::Usage::MAP_SWAP) == modules::graphics::Usage::MAP_SWAP) {
					if (auto forceSwap = (expectMapUsage & modules::graphics::Usage::MAP_FORCE_SWAP) == modules::graphics::Usage::MAP_FORCE_SWAP; forceSwap || buf->isSyncing()) {
						auto needCreate = true;
						if (forceSwap) {
							if (_count >= _max) needCreate = false;
						} else if (_count > 1) {
							if (_cur->next->target->isSyncing()) {
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

								if constexpr (std::is_base_of_v<modules::graphics::IVertexBuffer, T>) {
									newBuf->setFormat(buf->getFormat());
								}

								auto usage = newBuf->map(expectMapUsage);
								if (usage != modules::graphics::Usage::NONE) usage |= modules::graphics::Usage::DISCARD;
								return usage;
							} else {
								_cur = _cur->next;
								return _cur->target->map(expectMapUsage);
							}
						} else {
							_cur = _cur->next;
							return _cur->target->map(expectMapUsage);
						}
					} else {
						return buf->map(expectMapUsage);
					}
				} else {
					return buf->map(expectMapUsage);
				}
			}

			return modules::graphics::Usage::NONE;
		}

		inline void AE_CALL unmap() {
			if (_cur) _cur->target->unmap();
		}

		inline uint32_t AE_CALL read(uint32_t offset, void* dst, uint32_t dstLen) {
			return _cur ? _cur->target->read(offset, dst, dstLen) : -1;
		}

		inline uint32_t AE_CALL write(uint32_t offset, const void* data, uint32_t length) {
			return _cur ? _cur->target->write(offset, data, length) : -1;
		}

		inline uint32_t AE_CALL update(uint32_t offset, const void* data, uint32_t length) {
			return _cur ? _cur->target->update(offset, data, length) : -1;
		}

		inline bool AE_CALL isSyncing() const {
			return _cur ? _cur->target->isSyncing() : false;
		}

		void AE_CALL destroy() {
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

		T* AE_CALL _createBuffer() {
			if constexpr (std::is_base_of_v<modules::graphics::IVertexBuffer, T>) {
				return _graphics->createVertexBuffer();
			} else if constexpr (std::is_base_of_v<modules::graphics::IIndexBuffer, T>) {
				return _graphics->createIndexBuffer();
			} else if constexpr (std::is_base_of_v<modules::graphics::IConstantBuffer, T>) {
				return _graphics->createConstantBuffer();
			} else {
				return nullptr;
			}
		}
	};


	class AE_FW_DLL MultipleVertexBuffer : public modules::graphics::IVertexBuffer {
	public:
		MultipleVertexBuffer(modules::graphics::IGraphicsModule& graphics, uint8_t max);
		virtual ~MultipleVertexBuffer();

		virtual bool AE_CALL isCreated() const override;
		virtual const void* AE_CALL getNative() const override;
		virtual bool AE_CALL create(uint32_t size, modules::graphics::Usage bufferUsage, const void* data = nullptr, uint32_t dataSize = 0) override;
		virtual uint32_t AE_CALL getSize() const override;
		virtual modules::graphics::Usage AE_CALL getUsage() const override;
		virtual modules::graphics::Usage AE_CALL map(modules::graphics::Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual uint32_t AE_CALL read(uint32_t offset, void* dst, uint32_t dstLen) override;
		virtual uint32_t AE_CALL write(uint32_t offset, const void* data, uint32_t length) override;
		virtual uint32_t AE_CALL update(uint32_t offset, const void* data, uint32_t length) override;
		virtual const modules::graphics::VertexFormat& AE_CALL getFormat() const override;
		virtual void AE_CALL setFormat(const modules::graphics::VertexFormat& format) override;
		//virtual void AE_CALL flush() override;
		virtual bool AE_CALL isSyncing() const override;
		virtual void AE_CALL destroy() override;

	private:
		modules::graphics::VertexFormat _format;

		MultipleBuffer<modules::graphics::IVertexBuffer> _base;
	};


	class AE_FW_DLL MultipleIndexBuffer : public modules::graphics::IIndexBuffer {
	public:
		MultipleIndexBuffer(modules::graphics::IGraphicsModule& graphics, uint8_t max);
		virtual ~MultipleIndexBuffer();

		virtual bool AE_CALL isCreated() const override;
		virtual const void* AE_CALL getNative() const override;
		virtual bool AE_CALL create(uint32_t size, modules::graphics::Usage bufferUsage, const void* data = nullptr, uint32_t dataSize = 0) override;
		virtual uint32_t AE_CALL getSize() const override;
		virtual modules::graphics::Usage AE_CALL getUsage() const override;
		virtual modules::graphics::Usage AE_CALL map(modules::graphics::Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual uint32_t AE_CALL read(uint32_t offset, void* dst, uint32_t dstLen) override;
		virtual uint32_t AE_CALL write(uint32_t offset, const void* data, uint32_t length) override;
		virtual uint32_t AE_CALL update(uint32_t offset, const void* data, uint32_t length) override;
		virtual modules::graphics::IndexType AE_CALL getFormat() const override;
		virtual void AE_CALL setFormat(modules::graphics::IndexType type) override;
		//virtual void AE_CALL flush() override;
		virtual bool AE_CALL isSyncing() const override;
		virtual void AE_CALL destroy() override;

	private:
		modules::graphics::IndexType _idxType;

		MultipleBuffer<modules::graphics::IIndexBuffer> _base;
	};


	class AE_FW_DLL MultipleConstantBuffer : public modules::graphics::IConstantBuffer {
	public:
		MultipleConstantBuffer(modules::graphics::IGraphicsModule& graphics, uint8_t max);
		virtual ~MultipleConstantBuffer();

		virtual bool AE_CALL isCreated() const override;
		virtual const void* AE_CALL getNative() const override;
		virtual bool AE_CALL create(uint32_t size, modules::graphics::Usage bufferUsage, const void* data = nullptr, uint32_t dataSize = 0) override;
		virtual uint32_t AE_CALL getSize() const override;
		virtual modules::graphics::Usage AE_CALL getUsage() const override;
		virtual modules::graphics::Usage AE_CALL map(modules::graphics::Usage expectMapUsage) override;
		virtual void AE_CALL unmap() override;
		virtual uint32_t AE_CALL read(uint32_t offset, void* dst, uint32_t dstLen) override;
		virtual uint32_t AE_CALL write(uint32_t offset, const void* data, uint32_t length) override;
		virtual uint32_t AE_CALL update(uint32_t offset, const void* data, uint32_t length) override;
		//virtual void AE_CALL flush() override;
		virtual bool AE_CALL isSyncing() const override;
		virtual void AE_CALL destroy() override;

	private:
		MultipleBuffer<modules::graphics::IConstantBuffer> _base;
	};


	class AE_FW_DLL IVertexBufferGetter : public Ref {
	public:
		virtual ~IVertexBufferGetter() {}

		virtual modules::graphics::IVertexBuffer* AE_CALL get(const std::string& name) const = 0;
	};


	class AE_FW_DLL VertexBufferCollection : public IVertexBufferGetter {
	public:
		~VertexBufferCollection();

		virtual modules::graphics::IVertexBuffer* AE_CALL get(const std::string& name) const override;
		void AE_CALL set(const std::string& name, modules::graphics::IVertexBuffer* buffer);
		inline void AE_CALL remove(const std::string& name) {
			_buffers.erase(name);
		}
		inline bool AE_CALL isEmpty() const {
			return _buffers.empty();
		}
		inline void AE_CALL clear() {
			_buffers.clear();
		}

	private:
		std::unordered_map<std::string, RefPtr<modules::graphics::IVertexBuffer>> _buffers;
	};
}