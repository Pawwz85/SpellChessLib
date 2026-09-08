#pragma once

#include <QLayout>

namespace BoardViewer {
    class SingleItemSquareLayout : public QLayout {
        Q_OBJECT
        QLayoutItem * item;
        QSize itemSize;

        void setItem(QLayoutItem * newItem);

        public:
        SingleItemSquareLayout(QWidget * parent = nullptr);
        virtual ~SingleItemSquareLayout();

        void addItem(QLayoutItem *item) override;
        Qt::Orientations expandingDirections() const override;
        bool hasHeightForWidth() const override;
        int heightForWidth(int w) const override;
        int count() const override;
        QLayoutItem *itemAt(int index) const override;
        QLayoutItem *takeAt(int index) override;
        QSize minimumSize() const override;
        QSize sizeHint() const override;
        void setGeometry(const QRect &rect) override;

    };
}

