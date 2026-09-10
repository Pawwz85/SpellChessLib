#include <gtest/gtest.h>

#include <BoardGrid/PromotionContextMenuOverlay.hpp>

namespace {

using Board::BoardOrientation;
using Board::BoardWidgetPoint;
using Board::BoardWidgetSize;
using Board::LogicalBoardPoint;
using Board::PromotionActionIndex;
using Board::PromotionContextMenuOverlay;

const BoardWidgetSize boardSize(800.f, 800.f);

BoardWidgetPoint visualPoint(unsigned int row, BoardOrientation orientation) {
    const float x = orientation == BoardOrientation::Normal ? 150.f : 650.f;
    return BoardWidgetPoint(x, 250.f + static_cast<float>(row) * 100.f);
}

PromotionActionIndex expectedAction(unsigned int row) {
    return static_cast<PromotionActionIndex>(row);
}

} // namespace

TEST(PromotionContextMenuOverlay, MapsWidgetPointsToVisualRowsInBothOrientations) {
    PromotionContextMenuOverlay overlay;
    overlay.setPosition(LogicalBoardPoint(1.5f, 4.f));
    overlay.setSize(Board::LogicalBoardSize(1.f, 4.f));

    for (const auto orientation : {BoardOrientation::Normal, BoardOrientation::Flipped}) {
        for (unsigned int row = 0; row < 4; ++row) {
            const auto action = overlay.actionIndexFromWidgetPoint(
                visualPoint(row, orientation), boardSize, orientation);
            ASSERT_TRUE(action.has_value());
            EXPECT_EQ(*action, expectedAction(row));
        }
    }
}

TEST(PromotionContextMenuOverlay, LogicalAndWidgetPointsResolveToTheSameVisualRow) {
    PromotionContextMenuOverlay overlay;
    overlay.setPosition(LogicalBoardPoint(1.5f, 4.f));
    overlay.setSize(Board::LogicalBoardSize(1.f, 4.f));

    for (const auto orientation : {BoardOrientation::Normal, BoardOrientation::Flipped}) {
        for (unsigned int row = 0; row < 4; ++row) {
            const BoardWidgetPoint widgetPoint = visualPoint(row, orientation);
            const LogicalBoardPoint logicalPoint = Board::toLogicalCoordinates(
                widgetPoint, boardSize, orientation);

            EXPECT_EQ(
                overlay.actionIndexFromLogicalPoint(logicalPoint, boardSize, orientation),
                overlay.actionIndexFromWidgetPoint(widgetPoint, boardSize, orientation));
        }
    }
}

TEST(PromotionContextMenuOverlay, RejectsPointsOutsideTheOverlay) {
    PromotionContextMenuOverlay overlay;
    overlay.setPosition(LogicalBoardPoint(1.5f, 4.f));
    overlay.setSize(Board::LogicalBoardSize(1.f, 4.f));

    EXPECT_FALSE(overlay.actionIndexFromWidgetPoint(
        BoardWidgetPoint(50.f, 250.f), boardSize, BoardOrientation::Normal).has_value());
    EXPECT_FALSE(overlay.actionIndexFromWidgetPoint(
        BoardWidgetPoint(150.f, 650.f), boardSize, BoardOrientation::Normal).has_value());
    EXPECT_FALSE(overlay.actionIndexFromWidgetPoint(
        BoardWidgetPoint(650.f, 250.f), boardSize, BoardOrientation::Normal).has_value());
}
