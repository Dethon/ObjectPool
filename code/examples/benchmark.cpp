#include <iostream>
#include <array>
#include <vector>
#include <memory>
#include <chrono>
#include <string>
#include <numeric>

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

	int op(int v) const {
		return std::accumulate(m_a.cbegin(), m_a.cend(), 0) * v;
	}
};

void printElapsed(const std::chrono::steady_clock::time_point& startTime, std::string_view message) {
	auto elapsedTime = std::chrono::steady_clock::now() - startTime;
	std::cout << message << std::chrono::duration <double, std::milli>(elapsedTime).count() << " ms" << std::endl;
}

int testObpool(size_t size) {
	auto pool = obpool::ObjectPool<Widget>{ size, 1 };

	auto startTime = std::chrono::steady_clock::now();
	auto vec = pool.acquireAmount(size, 2);
	printElapsed(startTime, "Obpool work array acquisition takes ");

	startTime = std::chrono::steady_clock::now();
	auto result = std::accumulate(vec.cbegin(), vec.cend(), 0, [](int a, const auto& b) {
		return a + b->op(3);
	});
	printElapsed(startTime, "Obpool full iteration takes ");
	return result;
}

int testNonContiguous(size_t size) {
	auto startTime = std::chrono::steady_clock::now();
	auto nonContiguous = std::vector<std::unique_ptr<Widget>>{};
	nonContiguous.reserve(size);
	for (auto i = 0; i < size; i++) {
		nonContiguous.emplace_back(std::make_unique<Widget>(2));
	}
	printElapsed(startTime, "NonContiguous work array creation takes ");

	startTime = std::chrono::steady_clock::now();
	auto result = std::accumulate(nonContiguous.cbegin(), nonContiguous.cend(), 0, [](int a, const auto& b) {
		return a + b->op(3);
	});
	printElapsed(startTime, "NonContiguous full iteration takes ");
	return result;
}

int testFastest(size_t size) {
	auto startTime = std::chrono::steady_clock::now();
	auto fastest = std::vector<Widget>{};
	fastest.resize(size, 2);
	printElapsed(startTime, "Fastest work array creation takes ");

	startTime = std::chrono::steady_clock::now();
	auto result = std::accumulate(fastest.cbegin(), fastest.cend(), 0, [](int a, const auto& b) {
		return a + b.op(3);
	});
	printElapsed(startTime, "Fastest full iteration takes ");
	return result;
}

int main() {
	auto size = 5000000;

	auto result = 0;
	result += testObpool(size);
	std::cout << std::endl;
	result += testNonContiguous(size);
	std::cout << std::endl;
	result += testFastest(size);
	std::cout << std::endl;
	std::cout << result << std::endl;

	return 0;
}
