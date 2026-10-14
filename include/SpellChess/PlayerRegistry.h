#pragma once

#include "PlayerEntry.h"
#include <memory>
#include <map>
#include <atomic>
#include <exception>
#include <string>

namespace SpellChessLib {

	/*
		class responsible for storage of all player entries hold by a program
	*/
	class PlayerRegistry_t {
		std::map<int, PlayerEntry> entries;
		std::atomic<int> idSeq = 0;

		int nextId();
		
	public:

		class EntryDoesNotExistError : public std::exception {
			std::string msg;
		public:
			EntryDoesNotExistError(int id) : msg("Player Entry with the id of " + std::to_string(id) + " doesn't exist") {};
			const char* what() const noexcept {
				return msg.c_str();
			}
		};

		class const_iterator {
			std::map<int, PlayerEntry>::const_iterator it;
		public:
			const_iterator(std::map<int, PlayerEntry>::const_iterator it) : it(it) {};
			const PlayerEntry& operator*();
			const PlayerEntry* operator->();
			const bool operator==(const const_iterator& other);
			const bool operator!=(const const_iterator& other);
			const_iterator operator++();
		};

		const_iterator cbegin() { return entries.cbegin(); };
		const_iterator cend() { return entries.cend(); };
		void registerEntry(PlayerEntry & entry);
		void eraseEntry(int id);
		const PlayerEntry & get(const int& id) const; 
		
	};

	extern std::unique_ptr<PlayerRegistry_t> PlayerRegistry;

}