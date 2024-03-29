#pragma once

#include "srk/modules/graphics/GraphicsModule.h"
#ifdef __cpp_lib_generic_unordered_lookup
#	include <unordered_map>
#else
#	include <map>
#endif

namespace srk {
	template<typename T>
	class MultipleBuffer : public modules::graphics::IObject {
	public:
		struct Node {
			IntrusivePtr<T> target;
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

		inline bool SRK_CALL isCreated() const {
			return _cur ? _cur->target->isCreated() : false;
		}

		inline const void* SRK_CALL getNative() const {
			return _cur ? _cur->target->getNative() : nullptr;
		}

		bool SRK_CALL create(size_t size, modules::graphics::Usage requiredUsage, modules::graphics::Usage preferredUsage, const void* data, size_t dataSize) {
			destroy();

			if (auto buf = _createBuffer(); buf) {
				if (buf->create(size, requiredUsage, preferredUsage, data, dataSize)) {
					_head = new Node();
					_head->target = buf;
					_head->next = _head;
					_count = 1;

					_cur = _head;
				} else {
					return false;
				}
			}

			return false;
		}

		inline size_t SRK_CALL getSize() const {
			return _cur ? _cur->target->getSize() : 0;
		}

		inline modules::graphics::Usage SRK_CALL getUsage() const {
			return _cur ? _cur->target->getUsage() : modules::graphics::Usage::NONE;
		}

		modules::graphics::Usage SRK_CALL map(modules::graphics::Usage expectMapUsage) {
			using namespace srk::enum_operators;

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

								newBuf->create(buf->getSize(), buf->getUsage(), modules::graphics::Usage::NONE);

								if constexpr (std::derived_from<T, modules::graphics::IVertexBuffer>) {
									newBuf->setStride(buf->getStride());
								} else if constexpr (std::derived_from<T, modules::graphics::IIndexBuffer>) {
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

		inline void SRK_CALL unmap() {
			if (_cur) _cur->target->unmap();
		}

		inline size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset) {
			return _cur ? _cur->target->read(dst, dstLen, offset) : -1;
		}

		inline size_t SRK_CALL write(const void* data, size_t length, size_t offset) {
			return _cur ? _cur->target->write(data, length, offset) : -1;
		}

		inline size_t SRK_CALL update(const void* data, size_t length, size_t offset) {
			return _cur ? _cur->target->update(data, length, offset) : -1;
		}

		inline size_t SRK_CALL copyFrom(size_t dstPos, const modules::graphics::IBuffer* src, const Box1uz& srcRange) {
			return _cur ? _cur->target->copyFrom(dstPos, src, srcRange) : -1;
		}

		inline bool SRK_CALL isSyncing() const {
			return _cur ? _cur->target->isSyncing() : false;
		}

		void SRK_CALL destroy() {
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

		inline Node* SRK_CALL getBegin() const {
			return _head;
		}

		inline Node* SRK_CALL getCurrent() const {
			return _cur;
		}

		inline uint8_t SRK_CALL getCount() const {
			return _count;
		}

	private:
		uint8_t _count;
		uint8_t _max;

		Node* _head;
		Node* _cur;

		IntrusivePtr<T> SRK_CALL _createBuffer() {
			if constexpr (std::derived_from<T, modules::graphics::IVertexBuffer>) {
				return _graphics->createVertexBuffer();
			} else if constexpr (std::derived_from<T, modules::graphics::IIndexBuffer>) {
				return _graphics->createIndexBuffer();
			} else if constexpr (std::derived_from<T, modules::graphics::IConstantBuffer>) {
				return _graphics->createConstantBuffer();
			} else {
				return nullptr;
			}
		}
	};


	class SRK_FW_DLL MultipleVertexBuffer : public modules::graphics::IVertexBuffer {
	public:
		MultipleVertexBuffer(modules::graphics::IGraphicsModule& graphics, uint8_t max);
		virtual ~MultipleVertexBuffer();

		virtual bool SRK_CALL isCreated() const override;
		virtual const void* SRK_CALL getNative() const override;
		virtual bool SRK_CALL create(size_t size, modules::graphics::Usage requiredUsage, modules::graphics::Usage preferredUsage, const void* data = nullptr, size_t dataSize = 0) override;
		virtual size_t SRK_CALL getSize() const override;
		virtual modules::graphics::Usage SRK_CALL getUsage() const override;
		virtual modules::graphics::Usage SRK_CALL map(modules::graphics::Usage expectMapUsage) override;
		virtual void SRK_CALL unmap() override;
		virtual size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset) override;
		virtual size_t SRK_CALL write(const void* data, size_t length, size_t offset) override;
		virtual size_t SRK_CALL update(const void* data, size_t length, size_t offset) override;
		virtual size_t SRK_CALL copyFrom(size_t dstPos, const modules::graphics::IBuffer* src, const Box1uz& srcRange) override;
		virtual bool SRK_CALL isSyncing() const override;
		virtual void SRK_CALL destroy() override;

		virtual size_t SRK_CALL getStride() const override;
		virtual void SRK_CALL setStride(size_t stride) override;

	private:
		size_t _stride;
		MultipleBuffer<modules::graphics::IVertexBuffer> _base;
	};


	class SRK_FW_DLL MultipleIndexBuffer : public modules::graphics::IIndexBuffer {
	public:
		MultipleIndexBuffer(modules::graphics::IGraphicsModule& graphics, uint8_t max);
		virtual ~MultipleIndexBuffer();

		virtual bool SRK_CALL isCreated() const override;
		virtual const void* SRK_CALL getNative() const override;
		virtual bool SRK_CALL create(size_t size, modules::graphics::Usage requiredUsage, modules::graphics::Usage preferredUsage, const void* data = nullptr, size_t dataSize = 0) override;
		virtual size_t SRK_CALL getSize() const override;
		virtual modules::graphics::Usage SRK_CALL getUsage() const override;
		virtual modules::graphics::Usage SRK_CALL map(modules::graphics::Usage expectMapUsage) override;
		virtual void SRK_CALL unmap() override;
		virtual size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset) override;
		virtual size_t SRK_CALL write(const void* data, size_t length, size_t offset) override;
		virtual size_t SRK_CALL update(const void* data, size_t length, size_t offset) override;
		virtual size_t SRK_CALL copyFrom(size_t dstPos, const modules::graphics::IBuffer* src, const Box1uz& srcRange) override;
		virtual modules::graphics::IndexType SRK_CALL getFormat() const override;
		virtual void SRK_CALL setFormat(modules::graphics::IndexType type) override;
		virtual bool SRK_CALL isSyncing() const override;
		virtual void SRK_CALL destroy() override;

	private:
		modules::graphics::IndexType _idxType;

		MultipleBuffer<modules::graphics::IIndexBuffer> _base;
	};


	class SRK_FW_DLL MultipleConstantBuffer : public modules::graphics::IConstantBuffer {
	public:
		MultipleConstantBuffer(modules::graphics::IGraphicsModule& graphics, uint8_t max);
		virtual ~MultipleConstantBuffer();

		virtual bool SRK_CALL isCreated() const override;
		virtual const void* SRK_CALL getNative() const override;
		virtual bool SRK_CALL create(size_t size, modules::graphics::Usage requiredUsage, modules::graphics::Usage preferredUsage, const void* data = nullptr, size_t dataSize = 0) override;
		virtual size_t SRK_CALL getSize() const override;
		virtual modules::graphics::Usage SRK_CALL getUsage() const override;
		virtual modules::graphics::Usage SRK_CALL map(modules::graphics::Usage expectMapUsage) override;
		virtual void SRK_CALL unmap() override;
		virtual size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset) override;
		virtual size_t SRK_CALL write(const void* data, size_t length, size_t offset) override;
		virtual size_t SRK_CALL update(const void* data, size_t length, size_t offset) override;
		virtual size_t SRK_CALL copyFrom(size_t dstPos, const modules::graphics::IBuffer* src, const Box1uz& srcRange) override;
		virtual bool SRK_CALL isSyncing() const override;
		virtual void SRK_CALL destroy() override;

	private:
		MultipleBuffer<modules::graphics::IConstantBuffer> _base;
	};


	class SRK_FW_DLL VertexAttributeCollection : public modules::graphics::IVertexAttributeGetter {
	public:
		virtual ~VertexAttributeCollection();

		virtual std::optional<modules::graphics::VertexAttribute<modules::graphics::IVertexBuffer>> SRK_CALL get(const std::string_view& name) const override;
		void SRK_CALL set(const std::string_view& name, const modules::graphics::VertexAttribute<modules::graphics::IVertexBuffer>& attrib);

		inline std::optional<modules::graphics::VertexAttribute<modules::graphics::IVertexBuffer>> SRK_CALL remove(const std::string_view& name) {
			return _remove(name);
		}
		inline bool SRK_CALL isEmpty() const {
			return _views.empty();
		}
		inline void SRK_CALL clear() {
			_views.clear();
		}

	private:
#ifdef __cpp_lib_generic_unordered_lookup
		struct MapHasher {
			using is_transparent = void;
			template<typename K> inline size_t SRK_CALL operator()(K&& key) const { return std::hash<std::string_view>{}(key); }
		};
		std::unordered_map<std::string, modules::graphics::VertexAttribute<modules::graphics::IVertexBuffer>, MapHasher, std::equal_to<>>
#else
		std::map<std::string, modules::graphics::VertexAttribute<modules::graphics::IVertexBuffer>, std::less<>>
#endif
		_views;

		std::optional<modules::graphics::VertexAttribute<modules::graphics::IVertexBuffer>> SRK_CALL _remove(const std::string_view& name);
	};
}