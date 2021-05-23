#pragma once

#include "aurora/TaggedPtr.h"

namespace aurora::lockfree {
	enum class QueueMode : uint8_t {
		SPSC,
		MPSC,
		SPMC,
		MPMC
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
		bool _enqueue(I&& item) {
			auto n = new Node(std::forward<I>(item));
			if (!n) return false;

			auto oldTail = _tail;
			_tail = n;
			oldTail->next = n;

			return true;
		}

	public:
		using ValueType = T;

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

		inline bool enqueue(const T& item) {
			return _enqueue(item);
		}

		inline bool enqueue(T&& item) {
			return _enqueue(item);
		}

		bool dequeue(T& out) {
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
		bool _enqueue(I&& item) {
			auto n = new Node(std::forward<I>(item));
			if (!n) return false;

			auto oldTail = _tail.exchange(n, std::memory_order::acquire);
			oldTail->next.store(n, std::memory_order::release);

			return true;
		}

	public:
		using ValueType = T;

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

		inline bool enqueue(const T& item) {
			return _enqueue(item);
		}

		inline bool enqueue(T&& item) {
			return _enqueue(item);
		}

		bool dequeue(T& out) {
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
		bool _enqueue(I&& item) {
			auto n = new Node(std::forward<I>(item));
			if (!n) return false;

			auto oldTail = _tail;
			_tail = n;
			oldTail->next.store(n, std::memory_order::release);

			return true;
		}

	public:
		using ValueType = T;

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

		inline bool enqueue(const T& item) {
			return _enqueue(item);
		}

		inline bool enqueue(T&& item) {
			return _enqueue(item);
		}

		bool dequeue(T& out) {
			TaggedPtr<Node> head = _head.load(std::memory_order::acquire), newHead;
			Node* poped;

			do {
				poped = head.getPtr()->next.load(std::memory_order::acquire);
				if (!poped) return false;

				newHead.set(poped, head.getTag() + 1);
			} while (!_head.compare_exchange_strong(head, newHead, std::memory_order::release, std::memory_order::relaxed));

			out = std::move(poped->item);
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
		std::atomic<Node*> _tail;

		std::mutex _mtx;

		template<typename I>
		bool _enqueue(I&& item) {
			auto n = new Node(std::forward<I>(item));
			if (!n) return false;

			auto oldTail = _tail.exchange(n, std::memory_order::acquire);
			oldTail->next.store(n, std::memory_order::release);

			return true;
		}

	public:
		using ValueType = T;

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

		inline bool enqueue(const T& item) {
			return _enqueue(item);
		}

		inline bool enqueue(T&& item) {
			return _enqueue(item);
		}

		bool dequeue(T& out) {
			//std::scoped_lock lock(_mtx);

			TaggedPtr<Node> head = _head.load(), newHead;
			Node* poped;
			T val;

			size_t i = 0;
			do {
				++i;
				poped = head.getPtr()->next.load();
				if (!poped) return false;
				if (poped->item == 3722304989) {
					int a = 1;
				}

				val = poped->item;
				newHead.set(poped, head.getTag() + 1);
			} while (!_head.compare_exchange_strong(head, newHead));

			out = std::move(poped->item);
			if (out > 1000000) {
				int a = 1;
			}
			if ((uint64_t)newHead.getPtr()->next.load() == 0xddddddddddddddddULL) {
				int a = 1;
			}
			if (newHead.getPtr()->item == 3722304989) {
				int a = 1;
			}
			delete head.getPtr();

			return true;
		}
	};
}