#pragma once

#include "aurora/TaggedPtr.h"

namespace aurora::lockfree {
	enum class QueueMode : uint8_t {
		SPSC = 0b00,
		MPSC = 0b10,
		SPMC = 0b01,
		MPMC = 0b11
	};

	template<typename T, QueueMode Mode>
	class LinkedQueue;

	template<typename T>
	class LinkedQueue<T, QueueMode::SPSC> {
	private:
		struct Node {
			volatile Node* next;
			T item;

			Node() :
				next(nullptr) {
			}

			explicit Node(const T& item) :
				next(nullptr),
				item(item) {
			}

			explicit Node(T&& item) :
				next(nullptr),
				item(std::move(item)) {
			}
		};

		Node* _head;
		Node* _tail;

		template<typename I>
		bool _push(I&& item) {
			auto n = new Node(std::forward<I>(item));
			if (!n) return false;

			auto oldTail = _tail;
			_tail = n;
			std::atomic_thread_fence(std::memory_order::release);
			oldTail->next = n;

			return true;
		}

	public:
		using ValueType = T;
		static constexpr QueueMode MODE = QueueMode::SPSC;

		LinkedQueue() :
			_head(new Node()),
			_tail(_head) {

		}

		~LinkedQueue() {
			while (_head) {
				auto n = _head;
				_head = _head->next;

				delete n;
			}
		}

		inline bool push(const T& item) {
			return _push(item);
		}

		inline bool push(T&& item) {
			return _push(item);
		}

		bool pop(T& out) {
			auto poped = _head->next;
			if (!poped) return false;

			out = std::move(poped->item);

			auto oldHead = _head;
			_head = poped;
			_head->item = T();
			delete oldHead;

			return true;
		}
	};

	template<typename T>
	class LinkedQueue<T, QueueMode::MPSC> {
	private:
		struct Node {
			volatile std::atomic<Node*> next;
			T item;

			Node() :
				next(nullptr) {
			}

			explicit Node(const T& item) :
				next(nullptr),
				item(item) {
			}

			explicit Node(T&& item) :
				next(nullptr),
				item(std::move(item)) {
			}
		};

		Node* _head;
		std::atomic<Node*> _tail;

		template<typename I>
		bool _push(I&& item) {
			auto n = new Node(std::forward<I>(item));
			if (!n) return false;

			auto oldTail = _tail.exchange(n, std::memory_order::acquire);
			oldTail->next.store(n, std::memory_order::release);

			return true;
		}

	public:
		using ValueType = T;
		static constexpr QueueMode MODE = QueueMode::MPSC;

		LinkedQueue() :
			_head(new Node()),
			_tail(_head) {

		}

		~LinkedQueue() {
			while (_head) {
				auto n = _head;
				_head = _head->next.load(std::memory_order::acquire);

				delete n;
			}
		}

		inline bool push(const T& item) {
			return _push(item);
		}

		inline bool push(T&& item) {
			return _push(item);
		}

		bool pop(T& out) {
			auto poped = _head->next.load(std::memory_order::acquire);
			if (!poped) return false;

			out = std::move(poped->item);

			auto oldHead = _head;
			_head = poped;
			_head->item = T();
			delete oldHead;

			return true;
		}
	};

	template<typename T>
	class LinkedQueue<T, QueueMode::SPMC> {
	private:
		struct Node {
			volatile std::atomic<Node*> next;
			T item;

			Node() :
				next(nullptr) {
			}

			explicit Node(const T& item) :
				next(nullptr),
				item(item) {
			}

			explicit Node(T&& item) :
				next(nullptr),
				item(std::move(item)) {
			}
		};

		std::atomic<TaggedPtr<Node>> _head;
		Node* _tail;

		template<typename I>
		bool _push(I&& item) {
			auto n = new Node(std::forward<I>(item));
			if (!n) return false;

			auto oldTail = _tail;
			_tail = n;
			oldTail->next.store(n, std::memory_order::release);

			return true;
		}

	public:
		using ValueType = T;
		static constexpr QueueMode MODE = QueueMode::SPMC;

		LinkedQueue() {
			auto n = new Node();
			_head.store(n, std::memory_order::relaxed);
			_tail = n;
		}

		~LinkedQueue() {
			auto n = _head.load(std::memory_order::acquire).getPtr();
			while (n) {
				auto tmp = n;
				n = n->next.load(std::memory_order::acquire);

				delete tmp;
			}
		}

		inline bool push(const T& item) {
			return _push(item);
		}

		inline bool push(T&& item) {
			return _push(item);
		}

		bool pop(T& out) {
			TaggedPtr<Node> head;

			do {
				head = _head.load(std::memory_order::acquire);
				auto poped = head.getPtr()->next.load(std::memory_order::acquire);
				if (!poped) return false;

				if (head == _head.load(std::memory_order::acquire)) {
					out = poped->item;
					if (_head.compare_exchange_strong(head, TaggedPtr(poped, head.getTag() + 1), std::memory_order::release, std::memory_order::relaxed)) break;
				}
			} while (true);

			delete head.getPtr();

			return true;
		}
	};

	template<typename T>
	class LinkedQueue<T, QueueMode::MPMC> {
	private:
		struct Node {
			volatile std::atomic<Node*> next;
			T item;
			std::atomic_size_t count = 2;

			Node() :
				next(nullptr) {
			}

			explicit Node(const T& item) :
				next(nullptr),
				item(item) {
			}

			explicit Node(T&& item) :
				next(nullptr),
				item(std::move(item)) {
			}

			~Node() {
				int a = 1;
			}

			void release() {
				if (count.fetch_sub(1) <= 1) {
					delete this;
				}
			}
		};

		std::atomic<TaggedPtr<Node>> _head;
		std::atomic<Node*> _tail;

		template<typename I>
		bool _push(I&& item) {
			auto n = new Node(std::forward<I>(item));
			if (!n) return false;

			auto oldTail = _tail.exchange(n, std::memory_order::acquire);
			oldTail->next.store(n, std::memory_order::release);

			return true;
		}

	public:
		using ValueType = T;
		static constexpr QueueMode MODE = QueueMode::MPMC;

		LinkedQueue() {
			auto n = new Node();
			_head.store(n, std::memory_order::relaxed);
			_tail.store(n, std::memory_order::relaxed);
		}

		~LinkedQueue() {
			auto n = _head.load(std::memory_order::acquire).getPtr();
			while (n) {
				auto tmp = n;
				n = n->next.load(std::memory_order::acquire);

				delete tmp;
			}
		}

		inline bool push(const T& item) {
			return _push(item);
		}

		inline bool push(T&& item) {
			return _push(item);
		}

		bool pop(T& out) {
			TaggedPtr<Node> head;

			do {
				head = _head.load(std::memory_order::acquire);
				auto poped = head.getPtr()->next.load(std::memory_order::acquire);
				if (!poped) return false;

				if (head == _head.load(std::memory_order::acquire)) {
					out = poped->item;
					if (_head.compare_exchange_strong(head, TaggedPtr(poped, head.getTag() + 1), std::memory_order::release, std::memory_order::relaxed)) break;
				}
			} while (true);

			delete head.getPtr();

			return true;
		}
	};
}