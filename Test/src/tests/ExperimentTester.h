#include "../BaseTester.h"
#include "Experiment.h"
#include <algorithm>
#include <iostream>
#include <ranges>
#include <string_view>

struct SSTT {
	using VV = int;
	static constexpr size_t val = 1;

	void aa() {  }

	template<bool T>
	void bb() {}
};

template<typename T>
concept IsAAA = requires(T t, bool b) {
	{ t.aa() } -> std::same_as<void>;
	//requires std::same_as<decltype(t.aa()), bool>;
	//{ T::val }->size_t;

	//std::same_as<decltype(T::val), size_t>;

	typename T::VV;

	
	//{ T::bb<true>() } -> std::same_as<void>;
};

template<IsAAA T>
void aaa() {

}

template<auto V>
void AE_CALL get_name() {
	printdln(__FUNCSIG__);
}

auto print11 = [](auto const& view) {
	printdln(view);
};

class ExperimentTester : public BaseTester {
public:
	virtual int32_t AE_CALL run() override {
		constexpr std::string_view hello{ "Hello C++ 20 !" };
		//std::ranges::for_each(hello | std::views::split(' '), print11);

		return 0;
	}
};