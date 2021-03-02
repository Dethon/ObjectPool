#pragma once

#include <vector>
#include <memory>
#include <concepts>

namespace objectPoolConcepts {
	template<class T, class... Args>
	concept Resetable = requires(T obj, Args... args) {
		{obj.reset(std::declval<Args...>())};
	};
}
struct EmptyPool : public std::bad_alloc {
	const char* what() const noexcept override { return "ObjectPool is empty"; }
};

template<class T>
class ObjectPool {
private:
	class PoolReturner {
	private:
		std::weak_ptr<ObjectPool<T>*> m_pool;

	public:
		PoolReturner() {}
		PoolReturner(std::weak_ptr<ObjectPool<T>*> pool) : m_pool(pool) {}
		void operator()(T* ptr) {
			if (auto pool_ptr = m_pool.lock(); pool_ptr) {
				(*pool_ptr)->release(ptr);
			}
		}
	};

	PoolReturner m_returner;
	std::vector<T> m_pool;
	std::vector<std::unique_ptr<T, PoolReturner>> m_reusables;
	std::shared_ptr<ObjectPool*> m_aliveFlagPtr;

public:
	ObjectPool(const ObjectPool<T> &) = delete;
	ObjectPool(ObjectPool<T>&&) = delete;
	ObjectPool<T>& operator=(const ObjectPool<T> &) = delete;
	ObjectPool<T>& operator=(ObjectPool<T> &&) = delete;

	template<class... Args> requires std::constructible_from<T, Args...>
	ObjectPool(size_t size, Args&& ... args) {
		m_aliveFlagPtr = std::make_shared<ObjectPool*>(this);
		m_returner = PoolReturner{ m_aliveFlagPtr };
		upsize(size, std::forward<Args>(args)...);
	}

	virtual ~ObjectPool() {}

	template<class... Args> requires objectPoolConcepts::Resetable<T, Args...>
	std::unique_ptr<T, PoolReturner> acquire(Args&& ... args) {
		auto reusable = acquire();
		reusable->reset(std::forward<Args>(args)...);
		return reusable;
	}

	template<class... Args>
	std::unique_ptr<T, PoolReturner> acquire(Args&& ... args) {
		if (m_reusables.empty()) {
			throw EmptyPool();
		}
		auto reusable = std::move(m_reusables.back());
		m_reusables.pop_back();
		return reusable;
 	}

	template<class... Args>
	bool resize(size_t newsize, Args&& ... args) {
		if (amountAvailable() == size()) {
			if (size() > newsize) {
				return downsize(newsize);
			}
			else if (size() < newsize) {
				return upsize(newsize, std::forward<Args>(args)...);
			}
		}
		return false;
	}

	size_t amountAvailable() noexcept {
		return m_reusables.size();
	}

	size_t size() noexcept {
		return m_pool.size();
	}

private:
	bool downsize(size_t newsize) {
		m_reusables.resize(newsize);
		m_pool.resize(newsize);
		return true;
	}

	template<class... Args> requires std::constructible_from<T, Args...>
	bool upsize(size_t newsize, Args&& ... args) {
		m_pool.reserve(newsize);
		m_reusables.reserve(newsize);
		m_reusables.clear();
		for (auto i = size(); i < newsize; i++) {
			m_pool.emplace_back(std::forward<Args>(args)...);
		}
		for (auto& element : m_pool) {
			m_reusables.emplace_back(&element, m_returner);
		}
		return true;
	}

	void release(T* reusable)  {
		m_reusables.emplace_back(reusable);
	}
};