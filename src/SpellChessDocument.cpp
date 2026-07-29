#include <SpellChess/SpellChessDocument.h>
#include <SpellChess/MoveGen.h> // for move legality check
#include <stdexcept>

SpellChessLib::SpellChessDocument::Node::Node(
    const GameState & state,
    Handle parent,
    SpellChessDocument & doc) :
    gameState(state), parent(parent)
    {
        registerId(doc);
    }

void
SpellChessLib::SpellChessDocument::Node::registerId(SpellChessDocument & document) {
    static std::atomic_int idSequence = 0;
    nodeId = idSequence.fetch_add(1);

    std::scoped_lock<std::mutex> guard(document.mut);
    document.idMap[nodeId] = this;
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
                this->root = std::make_shared<Node>(root, Handle(), *this);
                this->tail = this->root;
             };

SpellChessLib::SpellChessDocument::SpellChessDocument(const Annotation::GameAnnotations & annotations) :
            dirty(false),
            gameResult(annotations.terminationReason),
            tags(annotations.tagSection) {
                root = std::make_shared<Node>(annotations.root, Handle(), *this);
                const Annotation::AnnotatedSequence & sequence = annotations.annotatedSequence;
                initNodeFromAnnotations(root, nullptr, sequence.begin(), sequence.end());
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
        if (index == (size_t)Annotation::Contants::AnnotatedSequenceNodeType::Variation) {
            variations.push_back(&std::get<Annotation::AnnotatedSequence>(*it));
            continue;
        }

        // Annotated sequence element
        if (index == (size_t)Annotation::Contants::AnnotatedSequenceNodeType::Element) {
            const Annotation::AnnotatedSequenceElement & element = std::get<Annotation::AnnotatedSequenceElement>(*it);
            size_t element_index = element.index();
            std::shared_ptr<Node> child;
            switch (element_index)
            {
            case (size_t)Annotation::Contants::AnnotatedSequenceItemType::Comment: 
                node->comment = std::get<std::string>(element);
                break;
            
            case (size_t)Annotation::Contants::AnnotatedSequenceItemType::NumericAnnotationGlyph:
                break;
            
            case (size_t)Annotation::Contants::AnnotatedSequenceItemType::Move:
                SpellChessMove move = std::get<SpellChessMove>(element);
                child = makeChildNode(node, move);
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
            if(it == var->end() || it->index() == (size_t)Annotation::Contants::AnnotatedSequenceNodeType::Variation) 
                continue;

            const auto & element = std::get<Annotation::AnnotatedSequenceElement>(*it);

            if (element.index() != (size_t)Annotation::Contants::AnnotatedSequenceItemType::Move)
                continue;

            SpellChessMove move = std::get<SpellChessMove>(element);
            std::shared_ptr<Node> child = makeChildNode(parent, move);
            initNodeFromAnnotations(child, parent, ++it, var->end());
        };
    }
};

std::shared_ptr<SpellChessLib::SpellChessDocument::Node> 
SpellChessLib::SpellChessDocument::makeChildNode(std::shared_ptr<Node> node, const SpellChessMove & move) {
    std::scoped_lock<std::mutex> guard(node->mut);
    GameState gs = node->gameState;
    gs.makeMove(move);
    auto result = std::make_shared<Node>(gs, node, *this); 
    node->transitions.push_back({result, move});

    // if the parent node is tail, move it
    if (node == tail.lock()) 
        tail = result;
    
    dirty.store(true);
    return result;
};

bool 
SpellChessLib::SpellChessDocument::isDirty() const {
    return dirty.load();
};

bool 
SpellChessLib::SpellChessDocument::clearDirtyFlag() {
    return dirty.exchange(false);
};

bool
SpellChessLib::SpellChessDocument::isRoot(Handle node) const {
    return !node.expired() && node.lock() == root;
};

bool
SpellChessLib::SpellChessDocument::isTail(Handle node) const {
    return !node.expired() && node.lock() == tail.lock();
}

bool
SpellChessLib::SpellChessDocument::has(Handle node) const {
    auto node_ = tryHandle(node);
    return idMap.find(node_->getNodeId()) != idMap.end();
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
   return tryHandle(handle)->parent;
};

SpellChessLib::SpellChessDocument::Handle
SpellChessLib::SpellChessDocument::getNextNode(Handle handle) const {
    auto node = tryHandle(handle);
    std::scoped_lock<std::mutex> guard(node->mut);
    return node->transitions.empty() ? Handle() : node->transitions[0].child;
};

std::vector<SpellChessLib::SpellChessDocument::Handle>
SpellChessLib::SpellChessDocument::getChildren(Handle handle) const {
    std::vector<Handle> result;
    auto node = tryHandle(handle);
    std::scoped_lock<std::mutex> guard(node->mut);
    for(const auto & link : node->transitions)
        result.push_back(link.child);
    return result;
};

std::vector<SpellChessLib::SpellChessDocument::Arc>
SpellChessLib::SpellChessDocument::getArcs(Handle handle) const {
    std::vector<Arc> result;
    auto node = tryHandle(handle);
    std::scoped_lock<std::mutex> guard(node->mut);
    for(const auto & link : node->transitions)
        result.push_back({link.child, link.move});
    return result;
};

SpellChessLib::GameState
SpellChessLib::SpellChessDocument::getGameState(Handle handle) const {
    return tryHandle(handle)->gameState;
};

std::string
SpellChessLib::SpellChessDocument::getComment(Handle handle) const {
    return tryHandle(handle)->getComment();
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
    auto node = tryHandle(h);
    std::scoped_lock<std::mutex> guard(mut);
    
    auto it = idMap.find(node->getNodeId());
    if(it == idMap.end())
        throw BadHandle();

    return *it->second;
};

SpellChessLib::SpellChessDocument::Handle
SpellChessLib::SpellChessDocument::makeMove(Handle where, const SpellChessMove & move){
    Node & node = referenceNodeByHandle(where);
    Handle result = node.getChildByMove(move);

    // If there is already matching node, return it 
    if( !result.expired())
        return result;

    SpellChessLib::MoveGenerator gen(node.gameState);

    if(!gen.isMoveLegal(move))
        throw IllegalMoveError(node.gameState, move);

    // .. if not we have to create it
    return makeChildNode(node.shared_from_this(), move);
};

void 
SpellChessLib::SpellChessDocument::writeNodeToAnnotatedSequence(const Node & node, Annotation::AnnotatedSequence & seq) const {
    std::scoped_lock<std::mutex> guard(node.mut);

    if(!node.comment.empty())
        seq.appendComment(node.comment);

    if(node.transitions.empty())
        return;

    seq.appendMove(node.transitions[0].move);

    size_t debug_s = node.transitions.size();

    for (size_t i = 1; i < node.transitions.size(); ++i){
        Annotation::AnnotatedSequence variation;
        variation.appendMove(node.transitions[i].move);
        writeNodeToAnnotatedSequence(*node.transitions[i].child, variation);
        seq.appendVariation(std::move(variation));
    }

    writeNodeToAnnotatedSequence(*node.transitions[0].child, seq);      
};

std::shared_ptr<const SpellChessLib::SpellChessDocument::Node>
SpellChessLib::SpellChessDocument::tryHandle(Handle handle) const{
    if (auto node = handle.lock()) 
        return node;
    throw BadHandle();
}



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

