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
public:
	using ptr = std::unique_ptr<T, typename PoolReturner>;

private:
	PoolReturner m_returner;
	std::vector<T> m_pool;
	std::vector<ptr> m_reusables;
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
		resize(size, std::forward<Args>(args)...);
	}

	virtual ~ObjectPool() {}

	template<class... Args> requires objectPoolConcepts::Resetable<T, Args...>
	ptr acquire(Args&& ... args) {
		if (auto reusable = acquire(); reusable != nullptr) {
			reusable->reset(std::forward<Args>(args)...);
			return reusable;
		}
		return nullptr;
	}

	template<class... Args>
	ptr acquire(Args&& ... args) {
		if (m_reusables.empty()) {
			return nullptr;
		}
		auto reusable = std::move(m_reusables.back());
		m_reusables.pop_back();
		return reusable;
 	}

	template<class... Args> requires std::constructible_from<T, Args...>
	bool resize(size_t newsize, Args&& ... args) {
		if (!isBeingUsed()) {
			m_pool.resize(newsize, std::forward<Args>(args)...);
			m_reusables.clear();
			m_reusables.reserve(newsize);
			for (auto& element : m_pool) {
				m_reusables.emplace_back(&element, m_returner);
			}
			m_pool.shrink_to_fit();
			m_reusables.shrink_to_fit();
			return true;
		}
		return false;
	}

	size_t amountAvailable() noexcept {
		return m_reusables.size();
	}

	size_t size() noexcept {
		return m_pool.size();
	}

	bool isBeingUsed() {
		return amountAvailable() != size();
	}

private:
	void release(T* reusable)  {
		m_reusables.emplace_back(reusable);
	}
};

template<class T>
using unique_ptr_pool = typename ObjectPool<T>::ptr;
