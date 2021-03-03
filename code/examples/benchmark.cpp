#include <iostream>
#include <array>
#include <vector>
#include <memory>
#include <chrono>
#include <string>

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
 		for (const auto& item : m_a) {
			v = (v + item) * v;
		}
		return v;
	}
};

void printElapsed(const std::chrono::steady_clock::time_point& startTime, std::string_view message) {
	auto elapsedTime = std::chrono::steady_clock::now() - startTime;
	std::cout << message << std::chrono::duration <double, std::milli>(elapsedTime).count() << " ms" << std::endl;
}

void testObpool(size_t size) {
	auto startTime = std::chrono::steady_clock::now();
	auto pool = obpool::ObjectPool<Widget>{ size, 1 };
	printElapsed(startTime, "obpool one time initialization takes ");

	startTime = std::chrono::steady_clock::now();
	auto vec = std::vector<obpool::unique_ptr_pool<Widget>>{};
	vec.reserve(size);
	while (pool.amountAvailable() > 0) {
		vec.push_back(pool.acquire(5));
	}
	printElapsed(startTime, "obpool work array generation takes ");

	startTime = std::chrono::steady_clock::now();
	auto result = std::vector<int>{};
	for (auto& item : vec) {
		result.push_back(item->op(1));
	}
	printElapsed(startTime, "obpool full iteration takes ");
}

void testNonContiguous(size_t size) {
	auto startTime = std::chrono::steady_clock::now();
	auto nonContiguous = std::vector<std::unique_ptr<Widget>>{};
	nonContiguous.reserve(size);
	for (auto i = 0; i < size; i++) {
		nonContiguous.push_back(std::make_unique<Widget>(5));
	}
	printElapsed(startTime, "Unpooled nonContiguous work array generation ");

	startTime = std::chrono::steady_clock::now();
	auto result = std::vector<int>{};
	for (auto& item : nonContiguous) {
		result.push_back(item->op(1));
	}
	printElapsed(startTime, "Unpooled nonContiguous full iteration takes ");
}

int main() {
	auto size = 500000;
	testObpool(size);
	std::cout << std::endl;
	testNonContiguous(size);
	
	return 0;
}
