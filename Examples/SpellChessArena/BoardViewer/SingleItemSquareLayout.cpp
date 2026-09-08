#include <SingleItemSquareLayout.hpp>

using namespace BoardViewer;

void SingleItemSquareLayout::setItem(QLayoutItem * newItem) {
    if (item)
        delete item;
    item = newItem;
}

SingleItemSquareLayout::SingleItemSquareLayout(QWidget * parent) : QLayout(parent), item(nullptr) {
    setContentsMargins(0, 0, 0, 0);
}

SingleItemSquareLayout::~SingleItemSquareLayout() {
    if (item)
        delete item;
}

void
SingleItemSquareLayout::addItem(QLayoutItem *item) {
    setItem(item);
}

bool 
SingleItemSquareLayout::hasHeightForWidth() const {
    return true;
}

int 
SingleItemSquareLayout::heightForWidth(int w) const {
    return w;
}


Qt::Orientations
SingleItemSquareLayout::expandingDirections() const {
    return Qt::Orientation::Horizontal |  Qt::Orientation::Vertical;
}

int 
SingleItemSquareLayout::count() const {
    return item == nullptr ? 0 : 1;
}

QLayoutItem *
SingleItemSquareLayout::itemAt(int index) const {
    return ( index == 0 ) ? item : nullptr;
}

QLayoutItem *
SingleItemSquareLayout::takeAt(int index) {
    QLayoutItem * result = nullptr;
    if (index == 0) {
        result = item;
        item = nullptr;
    }
    return result;
}

QSize
SingleItemSquareLayout::minimumSize() const {
    QSize size;
    if (item)
        size = size.expandedTo(item->minimumSize());
    
    int s = std::max(size.width(), size.height());
    size = QSize(s, s);
    const QMargins margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

QSize 
SingleItemSquareLayout::sizeHint() const {
    return minimumSize();
}

void 
SingleItemSquareLayout::setGeometry(const QRect &rect) {
    QLayout::setGeometry(rect);

    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int avalibleWidth = effectiveRect.width();
    int avalibleHeight = effectiveRect.height();
    int padding;
    int size;
    if (item) {
        padding = std::abs(avalibleHeight - avalibleWidth) / 2;
        size = std::min(avalibleHeight, avalibleWidth);
        if (avalibleHeight == avalibleWidth){
            item->setGeometry(effectiveRect);
        } else if (avalibleHeight > avalibleWidth){
            item->setGeometry(QRect(effectiveRect.left(), effectiveRect.top() + padding, size, size));
        } else {
            item->setGeometry(QRect(effectiveRect.left() + padding, effectiveRect.top(), size, size));
        }
    }

}

#include <moc_SingleItemSquareLayout.cpp>
