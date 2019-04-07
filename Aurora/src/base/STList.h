#pragma once

#include "base/LowLevel.h"
#include <functional>

namespace aurora {
	template<typename T>
	class AE_TEMPLATE_DLL STList {
	public:
		class Element {
		public:
			Element(T& value) :
				_valid(true),
				_value(value) {
			}

		private:
			bool _valid;
			T _value;

			friend class STList;
		};


		using iterator = typename std::list<Element>::iterator;
		using const_iterator = typename std::list<Element>::const_iterator;

		STList() :
			_validCount(0),
			_totalCount(0),
			_traversing(0) {
		}

		inline std::list<Element>& AE_CALL getRaw() const {
			return _raw;
		}

		inline ui32 AE_CALL size() const {
			return _validCount;
		}

		void AE_CALL pushBack(T& value) {
			++_validCount;
			++_totalCount;
			_raw.emplace_back(value);
		}

		const_iterator AE_CALL begin() const {
			return _raw.begin();
		}

		const_iterator AE_CALL end() const {
			return _raw.end();
		}

		void AE_CALL clear() {
			if (_validCount) {
				if (_traversing) {
					for (auto& e : _raw) e._valid = false;
				} else {
					_raw.clear();
					_totalCount = 0;
				}
				_validCount = 0;
			}
		}

		iterator AE_CALL find(T& value) {
			for (auto itr = _raw.begin(); itr != _raw.end(); ++itr) {
				auto& e = *itr;
				if (e._value == value && e._valid) return itr;
			}

			return _raw.end();
		}

		void AE_CALL erase(iterator& itr) {
			if ((*itr)._valid) --_validCount;

			if (_traversing) {
				(*itr)._valid = false;
			} else {
				--_totalCount;
				_raw.erase(itr);
			}
		}

		inline void AE_CALL beginTraverse() {
			++_traversing;
		}

		void AE_CALL traverse(const std::function<bool(T&)>& fn) {
			for (auto itr = _raw.begin(); itr != _raw.end(); ++itr) {
				auto& e = *itr;
				if (e._valid) {
					if (!fn(e._value)) break;
				}
			}
		}

		void AE_CALL traverse(const std::function<void(T&)>& fn) {
			for (auto itr = _raw.begin(); itr != _raw.end(); ++itr) {
				auto& e = *itr;
				if (e._valid) fn(e._value)
			}
		}

		void AE_CALL traverse(bool(*fn)(T&)) {
			for (auto itr = _raw.begin(); itr != _raw.end(); ++itr) {
				auto& e = *itr;
				if (e._valid) {
					if (!fn(e._value)) break;
				}
			}
		}

		void AE_CALL traverse(void(*fn)(T&)) {
			for (auto itr = _raw.begin(); itr != _raw.end(); ++itr) {
				auto& e = *itr;
				if (e._valid) fn(e._value)
			}
		}

		template<typename Class>
		void AE_CALL traverse(Class* target, bool(Class::*fn)(T&)) {
			for (auto itr = _raw.begin(); itr != _raw.end(); ++itr) {
				auto& e = *itr;
				if (e._valid) {
					if (!(target->*fn)(e._value)) break;
				}
			}
		}

		template<typename Class>
		void AE_CALL traverse(Class* target, void(Class::*fn)(T&)) {
			for (auto itr = _raw.begin(); itr != _raw.end(); ++itr) {
				auto& e = *itr;
				if (e._valid) (target->*fn)(e._value);
			}
		}

		inline void AE_CALL endTraverse() {
			if (--_traversing == 0 && _validCount != _totalCount) {
				for (auto itr = _raw.begin(); itr != _raw.end();) {
					auto& e = *itr;
					if (e._valid) {
						++itr;
					} else {
						itr = _raw.erase(itr);
					}
				}
				_totalCount = _validCount;
			}
		}

	protected:
		ui32 _traversing;
		ui32 _validCount;
		ui32 _totalCount;
		std::list<Element> _raw;
	};
}