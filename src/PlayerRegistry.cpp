#include <SpellChess/PlayerRegistry.h>
#include <SpellChess/PlayerEntry.h>
#include <utility>

std::unique_ptr<SpellChessLib::PlayerRegistry_t>  SpellChessLib::PlayerRegistry = std::make_unique<SpellChessLib::PlayerRegistry_t>();

int SpellChessLib::PlayerRegistry_t::nextId(){
    return idSeq++;
}

void SpellChessLib::PlayerRegistry_t::registerEntry(PlayerEntry& entry){
    entry.id = nextId();
    entries[entry.id] = entry;
}

void SpellChessLib::PlayerRegistry_t::eraseEntry(int id){
    auto it = entries.find(id);
    if (it == entries.end())
        throw EntryDoesNotExistError(id);
    entries.erase(it);
}

const SpellChessLib::PlayerEntry& SpellChessLib::PlayerRegistry_t::get(const int& id) const{
    auto it = entries.find(id);
    if (it == entries.end())
        throw EntryDoesNotExistError(id);
    return it->second;
}

const SpellChessLib::PlayerEntry& SpellChessLib::PlayerRegistry_t::const_iterator::operator*(){
    return it->second;
}

const SpellChessLib::PlayerEntry* SpellChessLib::PlayerRegistry_t::const_iterator::operator->() {
    return &it->second;
}


const bool SpellChessLib::PlayerRegistry_t::const_iterator::operator==(const const_iterator& other){
    return it == other.it;
}

const bool SpellChessLib::PlayerRegistry_t::const_iterator::operator!=(const const_iterator& other) {
    return it != other.it;
}

SpellChessLib::PlayerRegistry_t::const_iterator SpellChessLib::PlayerRegistry_t::const_iterator::operator++(){
    return it++;
}
