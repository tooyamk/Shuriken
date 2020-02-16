#pragma once

#include "aurora/Global.h"

namespace aurora {
	enum class StackPopperFlag : uint8_t {
		CHECK_POP = 0b1,
		MULTI_POP = 0b10
	};
	AE_DEFINE_ENUM_BIT_OPERATIION(StackPopperFlag);


	template<typename T, StackPopperFlag flags = (StackPopperFlag)0>
	class AE_TEMPLATE_DLL StackPopper {
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
	class AE_TEMPLATE_DLL StackPopper<T, StackPopperFlag::CHECK_POP> {
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
	class AE_TEMPLATE_DLL StackPopper<T, StackPopperFlag::MULTI_POP> {
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
	class AE_TEMPLATE_DLL StackPopper<T, StackPopperFlag::CHECK_POP | StackPopperFlag::MULTI_POP> {
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