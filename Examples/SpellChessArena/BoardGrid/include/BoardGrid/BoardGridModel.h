#pragma once
#include <QObject>
#include <cstdint>
#include <array>

 namespace Board {

    struct BoardGridConfig {
        uint32_t baseLightColour;
        uint32_t baseDarkColour;
        uint32_t baseHighlightedColour;
        uint32_t freezeAreaColour;
        uint32_t jumpSquareColour;
    };

    enum SquareProperty {
        Frozen,
        Jumpable,
        Interactable,
        Higlighted,
        HasMoveDot,
        SquareProperty_EnumSize
    };

    class BoardGridEditor ;

    extern BoardGridConfig makeDefaultBoardGridConfig();
    class BoardGridModel : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool reversed MEMBER m_reversed NOTIFY onReversed);
        friend BoardGridEditor;

        Board::BoardGridConfig config;
        std::array<uint64_t, SquareProperty_EnumSize> data;
        //uint64_t data[SquareProperty_EnumSize];
        public:
        BoardGridModel();
        
        BoardGridEditor startEdit();
        bool getPropertyValue(unsigned int index, SquareProperty property) const;
        inline const Board::BoardGridConfig & getConfig() const;
        unsigned int getIndex(unsigned int column, unsigned int row) const;
        void setConfig(const Board::BoardGridConfig & config);

        signals:
        void onReversed();
        void onConfigChanged();
        void onDataChanged();
        
        private:
        bool m_reversed;
    };  

    class BoardGridEditor {
        bool anyChange;
        std::array<uint64_t, SquareProperty_EnumSize> newData;
        Board::BoardGridModel * model;
        
        public:
        BoardGridEditor(Board::BoardGridModel* editedModel);
        BoardGridEditor & setPropertyBitmask(SquareProperty property, uint64_t bitmask);
        BoardGridEditor & setSquareProperty(SquareProperty property, unsigned int sqIndex, bool val);
        void commit();
    };

    const BoardGridConfig & BoardGridModel::getConfig() const {
        return config;
    };
 };