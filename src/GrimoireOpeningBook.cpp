#include <SpellChess/GrimoireOpeningBook.h>
#include <SpellChess/ZobristHash.h>
#include <random>

long SpellChessLib::GrimoireOpeningBook::count() const {
    if(fseek(file, 0, SEEK_END))
        return 0;
    return ftell(file) / 24;
};

long  SpellChessLib::GrimoireOpeningBook::findPosition(uint64_t hash) const {
    long start = 0;
    long end = count();
    long mid;
    // we are searching [start; end) range

    uint64_t currentHash;

    while (end != start) {
        mid = (start + end) / 2;
        currentHash = load_hash(mid);

        if (hash == currentHash) {
            if(mid && load_hash(mid - 1) != hash) 
                return mid; 
            end = mid;
            continue;
        };

        if (hash > currentHash)
            start = mid + 1;
        else
            end = mid;
         
    }

    if (start == count()) // could happen if the hash value is larger than any hash in the book 
        return -1;

    if (load_hash(start) == hash)
        return start;

    return -1;
};

long SpellChessLib::GrimoireOpeningBook::calculateFilePositionByIndex(long index) const {
    return 24L * index;
};

template <typename Number, int S>
Number numberFromBuffer(unsigned char * buff) {
    Number result = 0;
    for(int i = S; i > 0; ) {
        result <<= (Number)8;
        result |= (Number)buff[--i];
    }
    return result;
};

uint64_t SpellChessLib::GrimoireOpeningBook::load_hash(long index) const {
    unsigned char buffer[8];
    if (fseek(file, calculateFilePositionByIndex(index), SEEK_SET))
        return 0;

    if(fread(buffer, 1, 8, file) != 8)
        return 0;
    return numberFromBuffer<uint64_t, 8>(buffer);
};

SpellChessLib::PieceType decodePromotionTarget(char c) {
    switch (c)
    {
    case 1:
        return SpellChessLib::Knight;
    case 2:
        return SpellChessLib::Bishop;
    case 3:
        return SpellChessLib::Rook;
    case 4:
        return SpellChessLib::Queen;
    default:
        return SpellChessLib::None;
    }
};


SpellChessLib::GrimoireOpeningBook::entry SpellChessLib::GrimoireOpeningBook::loadEntry(long index) const {
    unsigned char buffer[24];
    fseek(file, calculateFilePositionByIndex(index), SEEK_SET);
    fread(buffer, 1, 24, file);
    
    SpellChessMove move;
    uint64_t hash       =   numberFromBuffer<uint64_t, 8>(buffer);
    uint16_t weight     =   numberFromBuffer<uint16_t, 2>(buffer + 8);
    uint16_t learn      =   numberFromBuffer<uint16_t, 2>(buffer + 10);
    move.from           =   buffer[12];
    move.to             =   buffer[13];
    move.freezeSquare   =   buffer[14];
    move.jumpSquare     =   buffer[15];
    move.promoteTo      =   decodePromotionTarget(buffer[16]);

    return {
        hash,
        weight,
        learn,
        move
    };
};

bool SpellChessLib::GrimoireOpeningBook::containsPosition(const GameState & state) const {
    uint64_t hash = calculateHash(state);
    std::lock_guard<std::mutex> guard(mtx);
    return findPosition(hash) != -1;
}

std::vector<SpellChessLib::GrimoireOpeningBook::entry> SpellChessLib::GrimoireOpeningBook::loadAllEntries(uint64_t hash) const {
    std::vector<entry> result;
    long pos = findPosition(hash);
    long size = count();

    if (pos == -1)
        return result;
    
    while (pos < size && load_hash(pos) == hash)
    {
        result.push_back(loadEntry(pos));
        pos++;
    }
    
    return result;
};

std::optional<SpellChessLib::SpellChessMove> SpellChessLib::GrimoireOpeningBook::pickRandomMove(const GameState & state) const{
    static std::random_device rand_device;
    std::lock_guard<std::mutex> guard(mtx);

    uint64_t hash = calculateHash(state);
    auto entries = loadAllEntries(hash);

    if (entries.empty())
        return std::nullopt;

    uint64_t cum_sum = 0;
    for (auto & entry : entries)
        cum_sum += entry.weight;
    
    std::uniform_int_distribution<uint64_t> dist(0, cum_sum - 1);
    uint64_t r = dist(rand_device);
    cum_sum = 0;

    for (auto & entry : entries) {
        cum_sum += entry.weight;
        if (r < cum_sum)
            return entry.move;
    };
    
    return std::nullopt;
};

std::optional<SpellChessLib::SpellChessMove> SpellChessLib::GrimoireOpeningBook::pickRandomUniformMove(const GameState & state) const {
    static std::random_device rand_device;
    std::lock_guard<std::mutex> guard(mtx);

    uint64_t hash = calculateHash(state);
    auto entries = loadAllEntries(hash);

    if (entries.empty())
        return std::nullopt;

    std::uniform_int_distribution<uint64_t> dist(0, entries.size() -1);
    uint64_t r = dist(rand_device);
    
    return entries[r].move;
};

SpellChessLib::GrimoireOpeningBook::~GrimoireOpeningBook() {
    (void)fclose(file);
}

SpellChessLib::GrimoireOpeningBook::GrimoireOpeningBook(const std::filesystem::path & path) {
    file = fopen(path.string().c_str(), "rb");

    if (!file) {
        throw OpeningBookLoader::BookNotFoundError(path);
    };

};

