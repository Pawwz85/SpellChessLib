#pragma once

#include "../Factory.h"
#include <memory>

namespace SpellChessLib {
	namespace Scheduler {
		
		template <class Object>
		class ResourcePointer;

		template <class Object>
		class ObjectAllocator {
			friend ResourcePointer<Object>;
		protected:
			virtual void release(Object*) = 0;
			virtual void destroy(Object*) = 0;
		public:
			virtual ~ObjectAllocator() = default;
			virtual ResourcePointer<Object> retrieve() = 0;
		
		};

		template <class Object>
		class ResourcePointer {
			std::shared_ptr<ObjectAllocator<Object>> allocator;
			Object* obj;
			void discardPointer();
			Object* release();
		public:
			ResourcePointer() {};
			ResourcePointer(std::nullptr_t _) : allocator(_), obj(_) {};
			ResourcePointer(Object* obj, std::shared_ptr<ObjectAllocator<Object>> allocator) : obj(obj), allocator(allocator) {}
			ResourcePointer(ResourcePointer<Object>&& other) noexcept: allocator(other.allocator), obj(other.obj) {
				other.allocator = nullptr;
				other.obj = nullptr;
			}
			virtual ~ResourcePointer() {
				discardPointer();
			};
			void reset();
			void realoc();
			Object* get() { return obj; };
			const Object* get() const { return obj; };
			Object& operator *() { return *obj; };
			const Object& operator *() const { return *obj; };
			Object* operator ->() { return obj; };
			const Object* operator->() const { return obj; };
			bool operator == (const ResourcePointer& other) const { return this->obj == other.obj; }
			bool operator == (std::nullptr_t _) const { return this->obj == nullptr; }
			bool operator != (const ResourcePointer& other) const { return this->obj != other.obj; }
			bool operator != (std::nullptr_t _) const { return this->obj != nullptr; }
			ResourcePointer& operator = (ResourcePointer<Object>&& other) noexcept;
		};

		template<class Object>
		class DefaultObjectAllocator : public ObjectAllocator<Object>, public std::enable_shared_from_this<DefaultObjectAllocator<Object>> {
			std::shared_ptr<Factory<Object>> factory;
		public:
			DefaultObjectAllocator(std::shared_ptr<Factory<Object>> fac) : factory(fac) {};
			ResourcePointer<Object> retrieve() override;
			void destroy(Object*) override;
			void release(Object*) override;
		};

		template<class Object>
		inline ResourcePointer<Object> DefaultObjectAllocator<Object>::retrieve(){
			std::unique_ptr<Object> obj = factory->makeObject();
			return ResourcePointer<Object>(obj.release(), this->shared_from_this());
		}

		template<class Object>
		inline void DefaultObjectAllocator<Object>::release(Object* obj){
			delete obj;
		}

		template<class Object>
		inline void DefaultObjectAllocator<Object>::destroy(Object* obj) {
			delete obj;
		}

		template<class Object>
		inline void ResourcePointer<Object>::discardPointer(){
			if (obj && allocator)
				allocator->release(obj);
		}

		template<class Object>
		inline Object* ResourcePointer<Object>::release(){
			Object* result = obj;
			obj = nullptr;
			return result;
		}

		template<class Object>
		inline void ResourcePointer<Object>::reset() {
			discardPointer();
			allocator = nullptr;
			obj = nullptr;
		}

		template<class Object>
		inline void ResourcePointer<Object>::realoc(){
			if (allocator){
				if(obj)
					this->allocator->destroy(obj);
				obj = this->allocator->retrieve().release();
			}
		}

		template<class Object>
		inline ResourcePointer<Object>& ResourcePointer<Object>::operator=(ResourcePointer<Object>&& other) noexcept
		{
			discardPointer();
			this->allocator = other.allocator;
			this->obj = other.obj;
			other.allocator = nullptr;
			other.obj = nullptr;
			return *this;
		}

		template <class Object>
		ResourcePointer<Object> makeResource(std::shared_ptr<Factory<Object>> factory) {
			auto a = std::make_shared<DefaultObjectAllocator<Object>>(factory);
			return a->retrieve();
		}
	}
}