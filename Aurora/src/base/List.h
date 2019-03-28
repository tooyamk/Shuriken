#pragma once

#include "base/LowLevel.h"
#include <functional>

namespace aurora {
	template<typename T>
	class List {
	public:
		class Element {
		private:
			Element(T& value) :
				_valid(true),
				_value(value) {
			}

			bool _valid;
			T _value;

		private:
			friend class List;
		};


		List() :
			_isTraversing(false) {
		}

		inline std::list<Element>& AE_CALL getRaw() const {
			return _raw;
		}

		void AE_CALL pushBack(T& value) {
			_raw.emplace_back(value);
		}

		std::list<Element>::iterator AE_CALL find(T& value) {
			for (auto itr = _raw.begin(); itr != _raw.end(); ++itr) {
				auto& e = *itr;
				if (e._value == value && e._valid) return itr;
			}

			return _raw.end();
		}

		void AE_CALL erase(std::list<Element>::iterator& itr) {
			if (_isTraversing) {

			} else {

			}
		}

		void AE_CALL traverse(const std::function<bool(T&)>& fn) {
			++_isTraversing;

			//todo

			--_isTraversing;
		}

	protected:
		bool _isTraversing;
		std::list<Element> _raw;
	};
}