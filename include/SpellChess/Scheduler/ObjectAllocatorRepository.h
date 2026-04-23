#pragma once

#include <memory>
#include <map>
#include <string>
#include "../Factory.h"
#include "PoolObjectAllocator.h"

namespace SpellChessLib {
	namespace Scheduler {

		struct ResourceAllocationReport {
			size_t inUse;
			size_t cached;
		};
		
		
		class AllocatorNotRegisteredException : public std::exception {
			std::string msg;
		public:
			AllocatorNotRegisteredException(int id) : msg("The repository can not access the allocator with the id of " + std::to_string(id) + " before its registration") {};
			const char* what() const noexcept {
				return msg.c_str();
			}
		};

		template <class Object>
		class ObjectPoolAllocatorRepository {
			std::map<int, std::shared_ptr<ObjectPoolAllocator<Object>>> allocators;

		public:
			using iterator = typename std::map<int, std::shared_ptr<ObjectPoolAllocator<Object>>>::iterator ;
			using const_iterator = typename std::map<int, std::shared_ptr<ObjectPoolAllocator<Object>>>::const_iterator;
			void registerAllocator(int id, std::shared_ptr<ObjectPoolAllocator<Object>> allocator);
			bool exists(int id) const;
			void erase(int id);
			void setPoolLimit(int id, size_t limit);
			ResourcePointer<Object> retrieveResource(int allocatorId);
			ResourceAllocationReport getResourceReport(int id) const; // Note: due to volatile nature of multithreaded environment, the obtained values should be threated as approximations
			iterator begin() { return allocators.begin(); };
			iterator end() { return allocators.end(); };
			const_iterator cbegin() const { return allocators.cbegin(); };
			const_iterator cend() const { return allocators.cend(); };
		};

		template<class Object>
		inline void ObjectPoolAllocatorRepository<Object>::registerAllocator(int id, std::shared_ptr<ObjectPoolAllocator<Object>> allocator){
			allocators[id] = allocator;
		}

		template<class Object>
		inline bool ObjectPoolAllocatorRepository<Object>::exists(int id) const{
			return allocators.find(id) != allocators.end();
		}

		template<class Object>
		inline void ObjectPoolAllocatorRepository<Object>::erase(int id){
			allocators.erase(id);
		}

		template<class Object>
		inline void ObjectPoolAllocatorRepository<Object>::setPoolLimit(int id, size_t limit){
			auto it = allocators.find(id);
			if (it == allocators.end())
				throw AllocatorNotRegisteredException(id);
			it->second->setMaxCacheSize(limit);
		}

		template<class Object>
		inline ResourcePointer<Object> ObjectPoolAllocatorRepository<Object>::retrieveResource(int allocatorId){
			auto it = allocators.find(allocatorId);
			if (it == allocators.end())
				throw AllocatorNotRegisteredException(allocatorId);
			return it->second->retrieve();
		}

		template<class Object>
		inline ResourceAllocationReport ObjectPoolAllocatorRepository<Object>::getResourceReport(int id) const
		{
			auto it = allocators.find(id);
			if (it == allocators.end())
				throw AllocatorNotRegisteredException(id);
			size_t cached = it->second->getObjectPoolSize();
			size_t total = it->second->getAliveObjectsCount();
			return {
				total > cached ? total - cached : 0,
				cached
			};
		}

		

	}
}