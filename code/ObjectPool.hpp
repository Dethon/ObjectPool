#pragma once

#include <vector>
#include <memory>
#include <concepts>

namespace obpool {
	template<class T> class ObjectPool;

	namespace concepts {
		template<class T, class... Args>
		concept Resetable = requires(T obj, Args... args) {
			{obj.reset(std::declval<Args...>())};
		};
	}

	template<class T>
	class PoolReturner {
	private:
		std::weak_ptr<ObjectPool<T>*> m_pool;
	public:
		PoolReturner(std::weak_ptr<ObjectPool<T>*> pool) : m_pool{ pool } {}
		void operator()(T* ptr) {
			if (auto pool_ptr = m_pool.lock(); pool_ptr) {
				(*pool_ptr)->release(ptr);
			}
		}
	};

	template<class T>
	using unique_ptr_pool = std::unique_ptr<T, PoolReturner<T>>;

	template<class T>
	class ObjectPool {
	private:
		friend class PoolReturner<T>;
		PoolReturner<T> m_returner;
		std::shared_ptr<ObjectPool<T>*> m_aliveFlagPtr;
		std::vector<T> m_pool;
		std::vector<unique_ptr_pool<T>> m_reusables;

	public:
		ObjectPool(const ObjectPool<T>&) = delete;
		ObjectPool(ObjectPool<T>&&) = delete;
		ObjectPool<T>& operator=(const ObjectPool<T>&) = delete;
		ObjectPool<T>& operator=(ObjectPool<T>&&) = delete;

		template<class... Args> requires std::constructible_from<T, Args...>
		ObjectPool(size_t size, Args&& ... args) : m_returner{ {} } {
			m_aliveFlagPtr = std::make_shared<ObjectPool<T>*>(this);
			m_returner = PoolReturner<T>{ m_aliveFlagPtr };
			resize(size, std::forward<Args>(args)...);
		}
		virtual ~ObjectPool() {
			m_aliveFlagPtr.reset();
		}

		template<class... Args> requires concepts::Resetable<T, Args...>
		unique_ptr_pool<T> acquire(Args&& ... args) {
			if (auto reusable = acquire(); reusable != nullptr) {
				reusable->reset(std::forward<Args>(args)...);
				return reusable;
			}
			else {
				return reusable;
			}
		}

		template<class... Args>
		unique_ptr_pool<T> acquire(Args&& ... args) {
			if (m_reusables.empty()) {
				return { nullptr, m_returner };
			}
			auto reusable = std::move(m_reusables.back());
			m_reusables.pop_back();
			return reusable;
		}

		template<class... Args> requires std::constructible_from<T, Args...>
		bool resize(size_t newsize, Args&& ... args) {
			if (!isBeingUsed()) {
				m_pool.clear();
				m_reusables.clear();
				m_pool.reserve(newsize);
				m_reusables.reserve(newsize);
				for (size_t i = 0; i < newsize; i++) {
					m_pool.emplace_back(std::forward<Args>(args)...);
				}
				for (auto& element : m_pool) {
					m_reusables.emplace_back(&element, m_returner);
				}
				return true;
			}
			return false;
		}

		size_t amountAvailable() const noexcept {
			return m_reusables.size();
		}

		size_t size() const noexcept {
			return m_pool.size();
		}

		bool isBeingUsed() const noexcept {
			return amountAvailable() != size();
		}

	private:
		void release(T* reusable) {
			m_reusables.emplace_back(reusable, m_returner);
		}
	};
}
