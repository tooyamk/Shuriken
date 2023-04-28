#pragma once

#include "srk/EnumOperators.h"

namespace srk {
	enum class StackPopperFlag : uint8_t {
		CHECK_POP = 0b1,
		MULTI_POP = 0b10
	};


	template<typename T, StackPopperFlag flags = (StackPopperFlag)0>
	class StackPopper {
	public:
		StackPopper(T& stack) :
			_stack(stack) {
		}

		~StackPopper() {
			_stack.pop();
		}

	private:
		T& _stack;
	};


	template<typename T>
	class StackPopper<T, StackPopperFlag::CHECK_POP> {
	public:
		StackPopper(T& stack, bool doPop) :
			_doPop(doPop),
			_stack(stack) {
		}

		~StackPopper() {
			if (_doPop) _stack.pop();
		}

	private:
		bool _doPop;
		T& _stack;
	};


	template<typename T>
	class StackPopper<T, StackPopperFlag::MULTI_POP> {
	public:
		StackPopper(T& stack, size_t count) :
			_count(count),
			_stack(stack) {
		}

		~StackPopper() {
			_stack.pop(_count);
		}

	private:
		size_t _count;
		T& _stack;
	};


	template<typename T>
	class StackPopper<T, enum_operators::operator|(StackPopperFlag::CHECK_POP, StackPopperFlag::MULTI_POP)> {
	public:
		StackPopper(T& stack, size_t count, bool doPop) :
			_doPop(doPop),
			_count(count),
			_stack(stack) {
		}

		~StackPopper() {
			if (_doPop) _stack.pop(_count);
		}

	private:
		bool _doPop;
		size_t _count;
		T& _stack;
	};
}