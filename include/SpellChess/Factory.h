#pragma once
#include <memory>

namespace SpellChessLib {

	template <class Object>
	class Factory {
	public:
		virtual std::unique_ptr<Object> makeObject() = 0;
	};

	template <class Object>
	class DefaultFactory : public Factory<Object> {
		public:
		std::unique_ptr<Object> makeObject() override {
			return std::make_unique<Object>();
		};
	};
}