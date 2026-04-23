#pragma once
#include <memory>

namespace SpellChessLib {

	template <class Object>
	class Factory {
	public:
		virtual std::unique_ptr<Object> makeObject() = 0;
	};

}