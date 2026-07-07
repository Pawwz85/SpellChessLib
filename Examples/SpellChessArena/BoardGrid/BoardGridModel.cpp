#include <BoardGridModel.h>
#include <QBrush>
#include <cstring>

using namespace Board;

constexpr uint32_t RGBA(unsigned int R, unsigned int G, unsigned int B, unsigned int A = 0xff) {
    return (A << 24) | (R << 16) | (G << 8) | B;
};

Board::BoardGridConfig Board::makeDefaultBoardGridConfig() {
    return {
        RGBA(0xd1, 0x8b, 0x47),
        RGBA(0xff, 0xce, 0x9e),
        RGBA(0xff, 0xff, 0x00),
        RGBA(0x00, 0x00, 0xd0, 0x7f),
        RGBA(0xff, 0xff, 0xff, 0x7f)
    };
};

BoardGridModel::BoardGridModel() : config(makeDefaultBoardGridConfig()), m_reversed(false) {
    data.fill(0);
};

unsigned int BoardGridModel::getIndex(unsigned int column, unsigned int row) const {
    if (column >= 8 || row >= 8)
        return (unsigned int)-1;
    if (m_reversed)
        row = 7 - row;
    return (row << 3) | column;
};

void BoardGridModel::setConfig(const Board::BoardGridConfig & config) {
    memcpy(&this->config, &config, sizeof(BoardGridConfig));
    onConfigChanged();
}


bool BoardGridModel::getPropertyValue(unsigned int index, SquareProperty property) const {
    if (index >= 64)
        return false; 
    uint64_t bit_mask = 1ull<<index;
    return data.at(property) & bit_mask;
};

BoardGridEditor BoardGridModel::startEdit() {
    return BoardGridEditor(this);
}


BoardGridEditor::BoardGridEditor(BoardGridModel * m) : model(m) {
    anyChange = false;
    for(size_t i = 0; i < m->data.size(); ++i) 
        newData[i] = model->data[i];
};

BoardGridEditor& BoardGridEditor::setPropertyBitmask(SquareProperty property, uint64_t bitmask) {
    anyChange = anyChange || (bitmask != model->data[property]);
    newData[property] = bitmask;
    return *this;
}

BoardGridEditor&  BoardGridEditor::setSquareProperty(SquareProperty property, unsigned int sqIndex, bool val) {
    uint64_t bitmask = 1ull << sqIndex;
    bool currentValue = model->data[property] & bitmask;
    anyChange = anyChange || (currentValue != val);

    if(val)
        newData[property] |= bitmask;
    else
        newData[property] &= ~bitmask;
    
    return *this;
}

void BoardGridEditor::commit() {
    if(anyChange) {
        for(size_t i = 0; i < newData.size(); ++i) {
            model->data[i] = newData[i];
        };
        model->onDataChanged();
        anyChange = false;
    }
};

Board::Perspective BoardGridModel::getPerspective() const {
    return m_reversed ? BlackPerspective : WhitePerspective;
};



#include <moc_BoardGridModel.cpp>