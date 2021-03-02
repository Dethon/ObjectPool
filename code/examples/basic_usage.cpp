#include <iostream>
#include "../ObjectPool.hpp"

class Widget
{
private:
	int m_a;

public:
	Widget() : m_a{ -1 } {}
	Widget(int v) {
		reset(v);
	}

	void reset(int v) {
		m_a = v;
	}
};

int main() {
	auto pool = ObjectPool<Widget>{ 3, 7 };
	auto obj1 = pool.acquire(2);
	auto obj2 = pool.acquire(2);
	auto obj3 = pool.acquire(2);
	auto r = pool.resize(10, 50);
	auto a = pool.amountAvailable();

	return 0;
}
