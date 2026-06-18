#include <SpellChess/SpellChessDocument.h>
#include <SpellChess/MoveGen.h> // for move legality check

SpellChessLib::SpellChessDocument::Node::Node(
    const GameState & state,
    Handle parent,
    SpellChessDocument * doc) :
    document(doc), gameState(state), parent(parent)
    {
        registerId();
    }

void
SpellChessLib::SpellChessDocument::Node::registerId() {
    static std::atomic_int idSequence = 0;
    nodeId = idSequence.fetch_add(1);

    std::scoped_lock<std::mutex> guard(document->mut);
    document->idMap[nodeId] = this;
};

const SpellChessLib::GameState & 
SpellChessLib::SpellChessDocument::Node::getGameState() const {
    return this->gameState;
};

std::string 
SpellChessLib::SpellChessDocument::Node::getComment() const {
    std::scoped_lock<std::mutex> guard(this->mut);
    return this->comment; // explicit copy
}

int 
SpellChessLib::SpellChessDocument::Node::getNodeId() const {
    return this->nodeId;
}

bool 
SpellChessLib::SpellChessDocument::Node::isLeaf() const {
    std::scoped_lock<std::mutex> guard(this->mut);
    return this->transitions.size() == 0;
};

const char * 
SpellChessLib::SpellChessDocument::BadHandle::what() const noexcept {
    return "Bad or expired node handle";
};

SpellChessLib::SpellChessDocument::IllegalMoveError::IllegalMoveError(const GameState & state, const SpellChessMove & move):
fen(state.toFen()), move(move) {};

const char * 
SpellChessLib::SpellChessDocument::IllegalMoveError::what() const noexcept {
    std::string result = "Move {" + spellChessMoveToString(this->move) + "} is illegal in position: " + this->fen;
};

SpellChessLib::SpellChessDocument::Handle 
SpellChessLib::SpellChessDocument::Node::getChildByMove(const SpellChessMove & move) const {
    std::scoped_lock<std::mutex> guard(this->mut);

    for (const auto & transition : transitions)
        if (transition.move == move)
            return transition.child;

    return Handle();
};

SpellChessLib::SpellChessDocument::Handle 
SpellChessLib::SpellChessDocument::Node::getFirstChildNode() const {
    std::scoped_lock<std::mutex> guard(this->mut);
    return transitions.empty() ? Handle() : transitions[0].child;
}

SpellChessLib::SpellChessDocument::Handle 
SpellChessLib::SpellChessDocument::Node::getParentNode() const {
    return parent;
};

SpellChessLib::SpellChessDocument::SpellChessDocument(const GameState & root,
             const Annotation::SevenTagRoster & roster,
             Annotation::GameTermination result) :
             dirty(false),
             gameResult(result),
             tags(roster)
             {
                this->root = std::make_shared<Node>(root, Handle(), this);
                this->tail = this->root;
             };

SpellChessLib::SpellChessDocument::SpellChessDocument(const Annotation::GameAnnotations & annotations) :
            dirty(false),
            gameResult(annotations.terminationReason),
            tags(annotations.tagSection) {
                root = std::make_shared<Node>(annotations.root, Handle(), this);

                const Annotation::AnnotatedSequence & sequence = annotations.annotatedSequence;
                initNodeFromAnnotations(root, nullptr, sequence.begin(), sequence.end());

                // find tail
                auto it = this->root;
                do {
                    this->tail = it;
                    it = it->transitions.size() ? it->transitions[0].child : nullptr; 
                } while(it != nullptr);

            }; 

void 
SpellChessLib::SpellChessDocument::initNodeFromAnnotations(std::shared_ptr<Node> node,
    std::shared_ptr<Node> parent,
    Annotation::AnnotatedSequence::const_iterator it,
    Annotation::AnnotatedSequence::const_iterator end) {
    std::vector<const Annotation::AnnotatedSequence*> variations;
    
    bool break_thy_loop = false;
    for (; it != end; ++it) {
        size_t index = it->index();

        // Annotated sequence (variation)
        if (index == 0) {
            variations.push_back(&std::get<Annotation::AnnotatedSequence>(*it));
            continue;
        }

        // Annotated sequence element
        if (index == 1) {
            const Annotation::AnnotatedSequenceElement & element = std::get<Annotation::AnnotatedSequenceElement>(*it);
            size_t element_index = element.index();
            std::shared_ptr<Node> child = nullptr;
            switch (element_index)
            {
            case 0: // comment
                node->comment = std::get<std::string>(element);
                break;
            
            case 1: // numeric annotation glyph
                break;
            
            case 2: // Move
                SpellChessMove move = std::get<SpellChessMove>(element);
                child = makeChildNode(node, move);
                node->transitions.push_back({child, move});
                initNodeFromAnnotations(child, node, ++it, end);
                break_thy_loop = true;
            }

        }

        if(break_thy_loop)
            break;
    };

    // Make sibling nodes for all variations
    if(parent != nullptr) {
        for (const Annotation::AnnotatedSequence * var : variations) {
            auto it = var->begin();

            // skip empty variations and variations that start with another variation 
            if(it == var->end() || it->index() != 1) 
                continue;

            const auto & element = std::get<Annotation::AnnotatedSequenceElement>(*it);

            if (element.index() != 2)
                continue;

            SpellChessMove move = std::get<SpellChessMove>(element);
            std::shared_ptr<Node> child = makeChildNode(parent, move);
            parent->transitions.push_back({child, move});
            initNodeFromAnnotations(child, parent, ++it, var->end());
        };
    }
    

};

std::shared_ptr<SpellChessLib::SpellChessDocument::Node> 
SpellChessLib::SpellChessDocument::makeChildNode(std::shared_ptr<Node> node, const SpellChessMove & move) {
    GameState gs = node->gameState;
    gs.makeMove(move);
    return std::make_shared<Node>(gs, node, this);
};

bool 
SpellChessLib::SpellChessDocument::isDirty() const {
    return dirty.load();
};

bool 
SpellChessLib::SpellChessDocument::clearDirtyFlag() {
    return dirty.exchange(false);
};

SpellChessLib::SpellChessDocument::Handle 
SpellChessLib::SpellChessDocument::getRoot() const {
    return root;
};

SpellChessLib::SpellChessDocument::Handle 
SpellChessLib::SpellChessDocument::getTail() const {
    std::scoped_lock<std::mutex> guard(mut);
    return tail;
};

SpellChessLib::SpellChessDocument::Handle
SpellChessLib::SpellChessDocument::getPreviousNode(Handle handle) const {
    if (auto node = handle.lock()) 
        return node->parent;
    throw BadHandle();    
};

SpellChessLib::SpellChessDocument::Handle
SpellChessLib::SpellChessDocument::getNextNode(Handle handle) const {
    if (auto node = handle.lock()) {
        std::scoped_lock<std::mutex> guard(node->mut);
        return node->transitions.empty() ? Handle() : node->transitions[0].child;
    } 
    throw BadHandle();
};

std::vector<SpellChessLib::SpellChessDocument::Handle>
SpellChessLib::SpellChessDocument::getChildren(Handle handle) const {
    std::vector<Handle> result;

    if (auto node = handle.lock()) {
        std::scoped_lock<std::mutex> guard(node->mut);
        for(const auto & link : node->transitions)
            result.push_back(link.child);
        return result;
    } 
    throw BadHandle();
};

SpellChessLib::GameState
SpellChessLib::SpellChessDocument::getGameState(Handle handle) const {
    if (auto node = handle.lock())
        return node->gameState;
    throw BadHandle();
};

std::string
SpellChessLib::SpellChessDocument::getComment(Handle handle) const {
    if (auto node = handle.lock()) 
        return node->getComment();
   throw BadHandle();
};

void
SpellChessLib::SpellChessDocument::setComment(Handle where, const std::string & comment) {
    Node & node = referenceNodeByHandle(where);
    std::scoped_lock<std::mutex> guard(node.mut);
    dirty.store(true);
    node.comment = comment;
};

SpellChessLib::SpellChessDocument::Node & 
SpellChessLib::SpellChessDocument::referenceNodeByHandle(Handle h) {
    std::shared_ptr<Node> result;
    if (auto node = h.lock()) {
        std::scoped_lock<std::mutex> guard(mut);
        auto it = idMap.find(node->getNodeId());

        if(it == idMap.end())
            throw BadHandle();

        return *it->second;
    };
    throw BadHandle();
};

SpellChessLib::SpellChessDocument::Handle
SpellChessLib::SpellChessDocument::makeMoveAndAdvance(Handle where, const SpellChessMove & move){
    Node & node = referenceNodeByHandle(where);
    Handle result = node.getChildByMove(move);

    // If there is already matching node, return it 
    if( !result.expired())
        return result;

    SpellChessLib::MoveGenerator gen(node.gameState);

    if(!gen.isMoveLegal(move))
        throw IllegalMoveError(node.gameState, move);

    // .. if not we have to create it
    std::scoped_lock<std::mutex> guard(node.mut);
    auto newNode = makeChildNode(node.shared_from_this(), move);
    node.transitions.push_back({newNode, move});

    // If node is tail, update tail reference
    if (node.shared_from_this() == tail.lock()) {
        std::scoped_lock<std::mutex> guard2(mut);
        tail = newNode;
    } 
    
    dirty.store(true);

    return newNode;

};

void 
SpellChessLib::SpellChessDocument::writeNodeToAnnotatedSequence(const Node & node, Annotation::AnnotatedSequence & seq) const {
    std::scoped_lock<std::mutex> guard(node.mut);

    if(!node.comment.empty())
        seq.pushBack(node.comment);

    if(node.transitions.empty())
        return;

    seq.pushBack(node.transitions[0].move);

    size_t debug_s = node.transitions.size();

    for (size_t i = 1; i < node.transitions.size(); ++i){
        Annotation::AnnotatedSequence variation;
        variation.pushBack(node.transitions[i].move);
        writeNodeToAnnotatedSequence(*node.transitions[i].child, variation);
        seq.pushBack(std::move(variation));
    }

    writeNodeToAnnotatedSequence(*node.transitions[0].child, seq);      
};

SpellChessLib::Annotation::GameAnnotations
SpellChessLib::SpellChessDocument::toAnnotations() const {
    Annotation::AnnotatedSequence seq;
    writeNodeToAnnotatedSequence(*root, seq);
    return {tags, seq, gameResult, root->gameState};
};

int
SpellChessLib::SpellChessDocument::getNodeId(Handle h) const {
    if (auto node = h.lock()) {
        return node->nodeId;
    } else 
        return -1;
};

SpellChessLib::SpellChessDocument::Handle
SpellChessLib::SpellChessDocument::getNodeById(int id) const {
    return idMap.at(id)->weak_from_this();
};

std::optional<std::string>
SpellChessLib::SpellChessDocument::getTag(const std::string & tagName) const {
    std::scoped_lock<std::mutex> guard(tagMut);
    using Result = typename std::optional<std::string>;
    auto where = tags.find(tagName);
    return (where != tags.end()) ? Result(where->value) : Result(std::nullopt);
};

void
SpellChessLib::SpellChessDocument::setTag(const std::string & tagName, const std::string & tagValue) {
    if (tagName == "Result") 
        throw std::domain_error("Manually setting 'Result' tag is prohibited. Use .setGameResult method instead");

    std::scoped_lock<std::mutex> guard(tagMut);
    tags[tagName] = tagValue;
    dirty.store(true);
};

void
SpellChessLib::SpellChessDocument::setTag(const Annotation::TagPair & tag) {
    setTag(tag.name, tag.value);
};

void
SpellChessLib::SpellChessDocument::setGameResult(Annotation::GameTermination result) {
    std::scoped_lock<std::mutex> guard(tagMut);
    gameResult = result;
    tags["Result"] =  Annotation::makeResultTag(result); 
    dirty.store(true);
};

std::shared_ptr<SpellChessLib::SpellChessDocument>
SpellChessLib::createEmptySpellChessDocument(const GameState & root) {
    auto roster = Annotation::beginSevenTagRooster("?", "?", "?");
    roster.black = roster.white = "?";
    roster.date = Annotation::makeCurrentDateTag();
    return std::make_shared<SpellChessDocument>(root, roster);
};


std::shared_ptr<SpellChessLib::SpellChessDocument>
SpellChessLib::createEmptySpellChessDocument(const std::string & fen) {
    FenParser parser;
    SpellChessFenValidator spellValidator;
    StandardChessFenValidator validator; 

    if(validator.isValidFen(fen.c_str())) 
        return createEmptySpellChessDocument(parser.fromStandardChessFen(fen));

    spellValidator.throwIfInvalid(fen.c_str());
    return createEmptySpellChessDocument(parser.fromSpellChessFen(fen));
};

