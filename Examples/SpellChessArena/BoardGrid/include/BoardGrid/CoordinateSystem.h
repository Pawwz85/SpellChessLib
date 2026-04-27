#pragma once

namespace Board {

    enum Perspective {
        WhitePerspective,
        BlackPerspective
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
    };

    using LogicalBoardPoint = typename Internal::Point<Internal::LogicalCoordynates>;
    using LogicalBoardSize = typename Internal::Size<Internal::LogicalCoordynates>;
    using BoardWidgetSize = typename Internal::Size<Internal::WidgetCoordynates>;

    class BoardWidgetPoint : public Internal::Point<Internal::WidgetCoordynates> {
            Perspective m_perspective;          
            public:
            BoardWidgetPoint() : Point(), m_perspective(WhitePerspective) {};
            BoardWidgetPoint(Perspective perspective, float x, float y): Point(x, y), m_perspective(perspective) {};
            BoardWidgetPoint(const BoardWidgetPoint & other) : Point(other), m_perspective(other.m_perspective) {};
            Perspective perspective() const {return m_perspective;};
    };

    inline LogicalBoardPoint logicalPointFromWidgetPoint(const BoardWidgetPoint & point, const BoardWidgetSize & boardSize) {
        float result_x = 8 * point.x() / boardSize.width();
        float result_y = 8 * point.y() / boardSize.height(); 
        
        if (point.perspective() == BlackPerspective)
            result_y = 8.f - result_y;

        return LogicalBoardPoint(result_x, result_y);
    };

    inline LogicalBoardSize logicalSizeFromWidgetSize(const BoardWidgetSize & size, const BoardWidgetSize & boardSize) {
        float width = 8 * size.width() / boardSize.width();
        float height = 8 * size.height() / boardSize.height();
        return LogicalBoardSize(width, height);
    };

};