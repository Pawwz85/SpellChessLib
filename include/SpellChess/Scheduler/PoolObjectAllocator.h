#pragma once

#include "ObjectAllocator.h"
#include <memory>
#include <queue>
#include <mutex>

namespace SpellChessLib {
	namespace Scheduler {

		template <class Object>
		class ObjectPoolAllocator : public ObjectAllocator<Object>, public std::enable_shared_from_this<ObjectPoolAllocator<Object>> {
			std::shared_ptr<Factory<Object>> factory;
			std::queue<Object*> objectPool;

			size_t totalAliveObjects = 0;
			size_t maxCacheSize = INT_MAX;

		protected:
			mutable std::mutex lock;
			void release(Object*) override;
			void destroy(Object* obj) override;
		public:
			ObjectPoolAllocator(std::shared_ptr<Factory<Object>> objectFactory) : factory(objectFactory) {};
			virtual ~ObjectPoolAllocator();

			ResourcePointer<Object> retrieve() override;

			void clearPool();
			void shrinkPool(const size_t& target);
			void setMaxCacheSize(const size_t& maxSize);
			const size_t getAliveObjectsCount() const;
			const size_t getObjectPoolSize() const;
		};

		template<class Object>
		inline void ObjectPoolAllocator<Object>::destroy(Object* obj){
			delete obj;
			--totalAliveObjects;
		}

		template<class Object>
		inline ObjectPoolAllocator<Object>::~ObjectPoolAllocator(){
			Object* ptr;
			while (!objectPool.empty()) {
				ptr = objectPool.front();
				objectPool.pop();
				destroy(ptr);
			}
		}

		template<class Object>
		inline ResourcePointer<Object> ObjectPoolAllocator<Object>::retrieve(){
			std::lock_guard<std::mutex> guard(lock);
			Object* result;
			if (!objectPool.empty()) {
				result = objectPool.front();
				objectPool.pop();
			}
			else {
				result = factory->makeObject().release();
				++totalAliveObjects;
			}
			return ResourcePointer<Object>(result, this->shared_from_this());
		}

		template<class Object>
		inline void ObjectPoolAllocator<Object>::release(Object* obj)
		{
			std::lock_guard<std::mutex> guard(lock);
			if (objectPool.size() < maxCacheSize)
				objectPool.push(obj);
			else
				destroy(obj);
		}

		template<class Object>
		inline void ObjectPoolAllocator<Object>::clearPool(){
			shrinkPool(0);
		}

		template<class Object>
		inline void ObjectPoolAllocator<Object>::shrinkPool(const size_t& target){
			std::lock_guard<std::mutex> guard(lock);
			Object* ptr;
			while (objectPool.size() > target) {
				ptr = objectPool.front();
				objectPool.pop();
				destroy(ptr);
			}
				
		}

		template<class Object>
		inline void ObjectPoolAllocator<Object>::setMaxCacheSize(const size_t& maxSize){
			std::lock_guard<std::mutex> guard(lock);
			maxCacheSize = maxSize;
			Object* ptr;
			while (objectPool.size() > maxSize) {
				ptr = objectPool.front();
				objectPool.pop();
				destroy(ptr);
			}
				
		}

		template<class Object>
		inline const size_t ObjectPoolAllocator<Object>::getAliveObjectsCount() const{
			std::lock_guard<std::mutex> guard(lock);
			return totalAliveObjects;
		}

		template<class Object>
		inline const size_t ObjectPoolAllocator<Object>::getObjectPoolSize() const{
			std::lock_guard<std::mutex> guard(lock);
			return objectPool.size();
		}

	}
}