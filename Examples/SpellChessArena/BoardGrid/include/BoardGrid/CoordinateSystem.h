#pragma once
#include <SpellChess/Board.h> // For SpellChessLib::NoSquare

namespace Board {

    enum class BoardOrientation {
        Normal,
        Flipped
    };

    namespace Internal {
        enum CoordinateSystemEnum {
            WidgetCoordynates,
            LogicalCoordynates
        };

        template <CoordinateSystemEnum cordSystem>
        class Point {
            float m_x;
            float m_y;
            public:
            Point() : m_x(0.f), m_y(0.f) {};
            Point(float x, float y) : m_x(x), m_y(y) {};
            Point(const Point<cordSystem> & other) : m_x(other.m_x), m_y(other.m_y) {};

            float x() const {return m_x;};
            float y() const {return m_y;};
        };

        template <CoordinateSystemEnum cordSystem>
        class Size {
            float m_width;
            float m_height;
            public:
            Size() : m_width(0.f), m_height(0.f) {};
            Size(float w, float h) : m_width(w), m_height(h) {};
            Size(const Size<cordSystem> & other) : m_width(other.m_width), m_height(other.m_height) {};

            float width() const {return m_width;};
            float height() const {return m_height;};
        };

        template <CoordinateSystemEnum cordSystem>
        class Rect {
            Point<cordSystem> m_position;
            Size<cordSystem> m_size;
            public:
            Rect() : {};
            Rect(float x, float y, float w, float h) : m_position(x, y), m_size(w, h) {};
            Rect(const Point<cordSystem> & pos, const Size<cordSystem> & size) : m_position(pos), m_size(size) {};
            Rect(const Rect<cordSystem> & other) : m_position(other.m_position), m_size(other.m_size) {};
            
            float x() const     {return m_position.x();};
            float y() const     {return m_position.y();};
            float left() const  {return x() - width()/2;};
            float right() const {return x() + width()/2;};
            float top() const   {return y() - height()/2;};
            float down() const  {return y() + height()/2;};
            float width() const {return  m_size.width();};
            float height() const{return m_size.height();};
            const Point<cordSystem> & position() const {return m_position;};
            const Size<cordSystem> & size() const {return m_size;}; 
        };
    };

    using LogicalBoardPoint = typename Internal::Point<Internal::LogicalCoordynates>;
    using LogicalBoardSize = typename Internal::Size<Internal::LogicalCoordynates>;
    using LogicalBoardRect = typename Internal::Rect<Internal::LogicalCoordynates>;

    using BoardWidgetPoint = typename Internal::Point<Internal::WidgetCoordynates>;
    using BoardWidgetSize = typename Internal::Size<Internal::WidgetCoordynates>;
    using BoardWidgetRect = typename Internal::Rect<Internal::WidgetCoordynates>;

    inline LogicalBoardPoint toLogicalCoordinates(const BoardWidgetPoint & point, const BoardWidgetSize & boardSize, BoardOrientation orientation) {
        float result_x = 8 * point.x() / boardSize.width();
        float result_y = 8 * point.y() / boardSize.height();

        if (orientation == BoardOrientation::Flipped) {
            result_x = 8. - result_x;
            result_y = 8. - result_y;
        };

        return LogicalBoardPoint(result_x, result_y);
    }

    inline LogicalBoardSize toLogicalCoordinates (const BoardWidgetSize & size , const BoardWidgetSize & boardSize, BoardOrientation orientation) {
        float result_w = 8 * size.width()  / boardSize.width();
        float result_h = 8 * size.height() / boardSize.height();

        return LogicalBoardSize(result_w, result_h);
    };

    inline LogicalBoardRect toLogicalCoordinates (const BoardWidgetRect & rect,  const BoardWidgetSize & boardSize, BoardOrientation orientation) {
        return LogicalBoardRect(
            toLogicalCoordinates(rect.position(), boardSize, orientation),
            toLogicalCoordinates(rect.size(), boardSize, orientation)
        );
    };

    inline BoardWidgetPoint toWidgetCoordinates(const LogicalBoardPoint & point, const BoardWidgetSize & boardSize, BoardOrientation orientation) {
        float result_x = point.x(), result_y = point.y();
        
        if (orientation == BoardOrientation::Flipped) {
            result_x = 8. - result_x;
            result_y = 8. - result_y;
        }

        result_x *= boardSize.width() / 8;
        result_y *= boardSize.height() / 8;

        return BoardWidgetPoint(result_x, result_y);
    }

    inline BoardWidgetSize toWidgetCoordinates(const LogicalBoardSize & size, const BoardWidgetSize & boardSize, BoardOrientation orientation) {
        float result_w = size.width(), result_h = size.height();
        result_w *= boardSize.width() / 8;
        result_h *= boardSize.height() / 8;
        return BoardWidgetSize(result_w, result_h);

    };

    inline BoardWidgetRect toWidgetCoordinates(const LogicalBoardRect & rect,  const BoardWidgetSize & boardSize, BoardOrientation orientation) {
         return BoardWidgetRect(
            toWidgetCoordinates(rect.position(), boardSize, orientation),
            toWidgetCoordinates(rect.size(), boardSize, orientation)
        );
    };

    inline unsigned int squareIndexFromDisplayCell(unsigned int column, unsigned int row, BoardOrientation orientation) {
        if (column >= 8 || row >= 8)
            return SpellChessLib::NoSquare;
        if (orientation == BoardOrientation::Flipped) {
            column = 7 - column;
            row = 7 - row;
        }
        return (row << 3) | column;
    };

    inline unsigned int squareIndexFromLogicalPoint(const LogicalBoardPoint & point) {
        if (point.x() < 0.f || point.y() < 0.f)
            return SpellChessLib::NoSquare;
        int column = static_cast<int>(point.x());
        int row = static_cast<int>(point.y());
        return squareIndexFromDisplayCell(static_cast<unsigned int>(column), static_cast<unsigned int>(row), BoardOrientation::Normal);
    };

    inline unsigned int squareIndexFromWidgetPoint(const BoardWidgetPoint & point, const BoardWidgetSize & boardSize, BoardOrientation orientation) {
        return squareIndexFromLogicalPoint(toLogicalCoordinates(point, boardSize, orientation));
    };

    inline LogicalBoardPoint squareCentreFromSquareIndex(unsigned int index) {
        if (index >= 64)
            return LogicalBoardPoint();

        int column = index & 7;
        int row =   index >> 3;

        return LogicalBoardPoint(column + 0.5f, row + 0.5f);
    }

};