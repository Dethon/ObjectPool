#include <array>
#include <vector>
#include <memory>
#include <iostream>

#include "../ObjectPool.hpp"

class Widget
{
private:
	std::array<int, 10> m_a{};
	std::array<int, 10> m_b{};
	std::array<int, 10> m_c{};

public:
	Widget(int v) {
		m_a.fill(v);
		m_b.fill(v);
		m_c.fill(v);
	}

	void reset(int v) {
		m_a.fill(v);
	}

	int op(int v) {
		int a = v;
		for (auto item : m_a) {
			a += item;
		}
		return a;
	}
};

int main() {
	auto pool = obpool::ObjectPool<Widget>{ 50, 0 };

	auto vec = std::vector<obpool::unique_ptr_pool<Widget>>{};
	vec.push_back(pool.acquire(5));
	vec.push_back(pool.acquire(7));

	auto result = std::vector<int>{};
	for (const auto& item : vec) {
		std::cout << item->op(10) << std::endl;
	}

	return 0;
}
