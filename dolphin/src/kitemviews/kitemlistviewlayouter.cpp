/***************************************************************************
 *   Copyright (C) 2011 by Peter Penz <peter.penz19@gmail.com>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "kitemlistviewlayouter_p.h"

#include "kitemmodelbase.h"
#include "kitemlistsizehintresolver_p.h"

#include <KDebug>

// #define KITEMLISTVIEWLAYOUTER_DEBUG

KItemListViewLayouter::KItemListViewLayouter(QObject* parent) :
    QObject(parent),
    m_dirty(true),
    m_visibleIndexesDirty(true),
    m_scrollOrientation(Qt::Vertical),
    m_size(),
    m_itemSize(128, 128),
    m_itemMargin(),
    m_headerHeight(0),
    m_model(0),
    m_sizeHintResolver(0),
    m_scrollOffset(0),
    m_maximumScrollOffset(0),
    m_itemOffset(0),
    m_maximumItemOffset(0),
    m_firstVisibleIndex(-1),
    m_lastVisibleIndex(-1),
    m_columnWidth(0),
    m_xPosInc(0),
    m_columnCount(0),
    m_groupItemIndexes(),
    m_groupHeaderHeight(0),
    m_groupHeaderMargin(0),
    m_itemInfos()
{
}

KItemListViewLayouter::~KItemListViewLayouter()
{
}

void KItemListViewLayouter::setScrollOrientation(Qt::Orientation orientation)
{
    if (m_scrollOrientation != orientation) {
        m_scrollOrientation = orientation;
        m_dirty = true;
    }
}

Qt::Orientation KItemListViewLayouter::scrollOrientation() const
{
    return m_scrollOrientation;
}

void KItemListViewLayouter::setSize(const QSizeF& size)
{
    if (m_size != size) {
        m_size = size;
        m_dirty = true;
    }
}

QSizeF KItemListViewLayouter::size() const
{
    return m_size;
}

void KItemListViewLayouter::setItemSize(const QSizeF& size)
{
    if (m_itemSize != size) {
        m_itemSize = size;
        m_dirty = true;
    }
}

QSizeF KItemListViewLayouter::itemSize() const
{
    return m_itemSize;
}

void KItemListViewLayouter::setItemMargin(const QSizeF& margin)
{
    if (m_itemMargin != margin) {
        m_itemMargin = margin;
        m_dirty = true;
    }
}

QSizeF KItemListViewLayouter::itemMargin() const
{
    return m_itemMargin;
}

void KItemListViewLayouter::setHeaderHeight(qreal height)
{
    if (m_headerHeight != height) {
        m_headerHeight = height;
        m_dirty = true;
    }
}

qreal KItemListViewLayouter::headerHeight() const
{
    return m_headerHeight;
}

void KItemListViewLayouter::setGroupHeaderHeight(qreal height)
{
    if (m_groupHeaderHeight != height) {
        m_groupHeaderHeight = height;
        m_dirty = true;
    }
}

qreal KItemListViewLayouter::groupHeaderHeight() const
{
    return m_groupHeaderHeight;
}

void KItemListViewLayouter::setGroupHeaderMargin(qreal margin)
{
    if (m_groupHeaderMargin != margin) {
        m_groupHeaderMargin = margin;
        m_dirty = true;
    }
}

qreal KItemListViewLayouter::groupHeaderMargin() const
{
    return m_groupHeaderMargin;
}

void KItemListViewLayouter::setScrollOffset(qreal offset)
{
    if (m_scrollOffset != offset) {
        m_scrollOffset = offset;
        m_visibleIndexesDirty = true;
    }
}

qreal KItemListViewLayouter::scrollOffset() const
{
    return m_scrollOffset;
}

qreal KItemListViewLayouter::maximumScrollOffset() const
{
    const_cast<KItemListViewLayouter*>(this)->doLayout();
    return m_maximumScrollOffset;
}

void KItemListViewLayouter::setItemOffset(qreal offset)
{
    if (m_itemOffset != offset) {
        m_itemOffset = offset;
        m_visibleIndexesDirty = true;
    }
}

qreal KItemListViewLayouter::itemOffset() const
{
    return m_itemOffset;
}

qreal KItemListViewLayouter::maximumItemOffset() const
{
    const_cast<KItemListViewLayouter*>(this)->doLayout();
    return m_maximumItemOffset;
}

void KItemListViewLayouter::setModel(const KItemModelBase* model)
{
    if (m_model != model) {
        m_model = model;
        m_dirty = true;
    }
}

const KItemModelBase* KItemListViewLayouter::model() const
{
    return m_model;
}

void KItemListViewLayouter::setSizeHintResolver(const KItemListSizeHintResolver* sizeHintResolver)
{
    if (m_sizeHintResolver != sizeHintResolver) {
        m_sizeHintResolver = sizeHintResolver;
        m_dirty = true;
    }
}

const KItemListSizeHintResolver* KItemListViewLayouter::sizeHintResolver() const
{
    return m_sizeHintResolver;
}

int KItemListViewLayouter::firstVisibleIndex() const
{
    const_cast<KItemListViewLayouter*>(this)->doLayout();
    return m_firstVisibleIndex;
}

int KItemListViewLayouter::lastVisibleIndex() const
{
    const_cast<KItemListViewLayouter*>(this)->doLayout();
    return m_lastVisibleIndex;
}

QRectF KItemListViewLayouter::itemRect(int index) const
{
    const_cast<KItemListViewLayouter*>(this)->doLayout();
    if (index < 0 || index >= m_itemInfos.count()) {
        return QRectF();
    }

    if (m_scrollOrientation == Qt::Horizontal) {
        // Rotate the logical direction which is always vertical by 90°
        // to get the physical horizontal direction
        const QRectF& b = m_itemInfos[index].rect;
        QRectF bounds(b.y(), b.x(), b.height(), b.width());
        QPointF pos = bounds.topLeft();
        pos.rx() -= m_scrollOffset;
        bounds.moveTo(pos);
        return bounds;
    }

    QRectF bounds = m_itemInfos[index].rect;
    bounds.moveTo(bounds.topLeft() - QPointF(m_itemOffset, m_scrollOffset));
    return bounds;
}

QRectF KItemListViewLayouter::groupHeaderRect(int index) const
{
    const_cast<KItemListViewLayouter*>(this)->doLayout();

    const QRectF firstItemRect = itemRect(index);
    QPointF pos = firstItemRect.topLeft();
    if (pos.isNull()) {
        return QRectF();
    }

    QSizeF size;
    if (m_scrollOrientation == Qt::Vertical) {
        pos.rx() = 0;
        pos.ry() -= m_groupHeaderHeight;
        size = QSizeF(m_size.width(), m_groupHeaderHeight);
    } else {
        pos.rx() -= m_itemMargin.width();
        pos.ry() = 0;

        // Determine the maximum width used in the
        // current column. As the scroll-direction is
        // Qt::Horizontal and m_itemRects is accessed directly,
        // the logical height represents the visual width.
        qreal width = minimumGroupHeaderWidth();
        const qreal y = m_itemInfos[index].rect.y();
        const int maxIndex = m_itemInfos.count() - 1;
        while (index <= maxIndex) {
            QRectF bounds = m_itemInfos[index].rect;
            if (bounds.y() != y) {
                break;
            }

            if (bounds.height() > width) {
                width = bounds.height();
            }

            ++index;
        }

        size = QSizeF(width, m_size.height());
    }
    return QRectF(pos, size);
}

int KItemListViewLayouter::itemColumn(int index) const
{
    const_cast<KItemListViewLayouter*>(this)->doLayout();
    if (index < 0 || index >= m_itemInfos.count()) {
        return -1;
    }

    return (m_scrollOrientation == Qt::Vertical)
            ? m_itemInfos[index].column
            : m_itemInfos[index].row;
}

int KItemListViewLayouter::itemRow(int index) const
{
    const_cast<KItemListViewLayouter*>(this)->doLayout();
    if (index < 0 || index >= m_itemInfos.count()) {
        return -1;
    }

    return (m_scrollOrientation == Qt::Vertical)
            ? m_itemInfos[index].row
            : m_itemInfos[index].column;
}

int KItemListViewLayouter::maximumVisibleItems() const
{
    const_cast<KItemListViewLayouter*>(this)->doLayout();

    const int height = static_cast<int>(m_size.height());
    const int rowHeight = static_cast<int>(m_itemSize.height());
    int rows = height / rowHeight;
    if (height % rowHeight != 0) {
        ++rows;
    }

    return rows * m_columnCount;
}

bool KItemListViewLayouter::isFirstGroupItem(int itemIndex) const
{
    const_cast<KItemListViewLayouter*>(this)->doLayout();
    return m_groupItemIndexes.contains(itemIndex);
}

void KItemListViewLayouter::markAsDirty()
{
    m_dirty = true;
}


#ifndef QT_NO_DEBUG
    bool KItemListViewLayouter::isDirty()
    {
        return m_dirty;
    }
#endif

void KItemListViewLayouter::doLayout()
{
    if (m_dirty) {
#ifdef KITEMLISTVIEWLAYOUTER_DEBUG
        QElapsedTimer timer;
        timer.start();
#endif
        m_visibleIndexesDirty = true;

        QSizeF itemSize = m_itemSize;
        QSizeF itemMargin = m_itemMargin;
        QSizeF size = m_size;

        const bool grouped = createGroupHeaders();

        const bool horizontalScrolling = (m_scrollOrientation == Qt::Horizontal);
        if (horizontalScrolling) {
            // Flip everything so that the layout logically can work like having
            // a vertical scrolling
            itemSize.setWidth(m_itemSize.height());
            itemSize.setHeight(m_itemSize.width());
            itemMargin.setWidth(m_itemMargin.height());
            itemMargin.setHeight(m_itemMargin.width());
            size.setWidth(m_size.height());
            size.setHeight(m_size.width());

            if (grouped) {
                // In the horizontal scrolling case all groups are aligned
                // at the top, which decreases the available height. For the
                // flipped data this means that the width must be decreased.
                size.rwidth() -= m_groupHeaderHeight;
            }
        }

        m_columnWidth = itemSize.width() + itemMargin.width();
        const qreal widthForColumns = size.width() - itemMargin.width();
        m_columnCount = qMax(1, int(widthForColumns / m_columnWidth));
        m_xPosInc = itemMargin.width();

        const int itemCount = m_model->count();
        if (itemCount > m_columnCount && m_columnWidth >= 32) {
            // Apply the unused width equally to each column
            const qreal unusedWidth = widthForColumns - m_columnCount * m_columnWidth;
            if (unusedWidth > 0) {
                const qreal columnInc = unusedWidth / (m_columnCount + 1);
                m_columnWidth += columnInc;
                m_xPosInc += columnInc;
            }
        }

        int rowCount = itemCount / m_columnCount;
        if (itemCount % m_columnCount != 0) {
            ++rowCount;
        }

        m_itemInfos.reserve(itemCount);

        qreal y = m_headerHeight + itemMargin.height();
        int row = 0;

        int index = 0;
        while (index < itemCount) {
            qreal x = m_xPosInc;
            qreal maxItemHeight = itemSize.height();

            if (grouped) {
                if (horizontalScrolling) {
                    // All group headers will always be aligned on the top and not
                    // flipped like the other properties
                    x += m_groupHeaderHeight;
                }

                if (m_groupItemIndexes.contains(index)) {
                    // The item is the first item of a group.
                    // Increase the y-position to provide space
                    // for the group header.
                    if (index > 0) {
                        // Only add a margin if there has been added another
                        // group already before
                        y += m_groupHeaderMargin;
                    } else if (!horizontalScrolling) {
                        // The first group header should be aligned on top
                        y -= itemMargin.height();
                    }

                    if (!horizontalScrolling) {
                        y += m_groupHeaderHeight;
                    }
                }
            }

            int column = 0;
            while (index < itemCount && column < m_columnCount) {
                qreal requiredItemHeight = itemSize.height();
                if (m_sizeHintResolver) {
                    const QSizeF sizeHint = m_sizeHintResolver->sizeHint(index);
                    const qreal sizeHintHeight = horizontalScrolling ? sizeHint.width() : sizeHint.height();
                    if (sizeHintHeight > requiredItemHeight) {
                        requiredItemHeight = sizeHintHeight;
                    }
                }

                const QRectF bounds(x, y, itemSize.width(), requiredItemHeight);
                if (index < m_itemInfos.count()) {
                    m_itemInfos[index].rect = bounds;
                    m_itemInfos[index].column = column;
                    m_itemInfos[index].row = row;
                } else {
                    ItemInfo itemInfo;
                    itemInfo.rect = bounds;
                    itemInfo.column = column;
                    itemInfo.row = row;
                    m_itemInfos.append(itemInfo);
                }

                if (grouped && horizontalScrolling) {
                    // When grouping is enabled in the horizontal mode, the header alignment
                    // looks like this:
                    //   Header-1 Header-2 Header-3
                    //   Item 1   Item 4   Item 7
                    //   Item 2   Item 5   Item 8
                    //   Item 3   Item 6   Item 9
                    // In this case 'requiredItemHeight' represents the column-width. We don't
                    // check the content of the header in the layouter to determine the required
                    // width, hence assure that at least a minimal width of 15 characters is given
                    // (in average a character requires the halve width of the font height).
                    //
                    // TODO: Let the group headers provide a minimum width and respect this width here
                    const qreal headerWidth = minimumGroupHeaderWidth();
                    if (requiredItemHeight < headerWidth) {
                        requiredItemHeight = headerWidth;
                    }
                }

                maxItemHeight = qMax(maxItemHeight, requiredItemHeight);
                x += m_columnWidth;
                ++index;
                ++column;

                if (grouped && m_groupItemIndexes.contains(index)) {
                    // The item represents the first index of a group
                    // and must aligned in the first column
                    break;
                }
            }

            y += maxItemHeight + itemMargin.height();
            ++row;
        }
        if (m_itemInfos.count() > itemCount) {
            m_itemInfos.erase(m_itemInfos.begin() + itemCount,
                              m_itemInfos.end());
        }

        if (itemCount > 0) {
            // Calculate the maximum y-range of the last row for m_maximumScrollOffset
            m_maximumScrollOffset = m_itemInfos.last().rect.bottom();
            const qreal rowY = m_itemInfos.last().rect.y();

            int index = m_itemInfos.count() - 2;
            while (index >= 0 && m_itemInfos[index].rect.bottom() >= rowY) {
                m_maximumScrollOffset = qMax(m_maximumScrollOffset, m_itemInfos[index].rect.bottom());
                --index;
            }

            m_maximumScrollOffset += itemMargin.height();

            m_maximumItemOffset = m_columnCount * m_columnWidth;
        } else {
            m_maximumScrollOffset = 0;
            m_maximumItemOffset = 0;
        }

#ifdef KITEMLISTVIEWLAYOUTER_DEBUG
        kDebug() << "[TIME] doLayout() for " << m_model->count() << "items:" << timer.elapsed();
#endif
        m_dirty = false;
    }

    updateVisibleIndexes();
}

void KItemListViewLayouter::updateVisibleIndexes()
{
    if (!m_visibleIndexesDirty) {
        return;
    }

    Q_ASSERT(!m_dirty);

    if (m_model->count() <= 0) {
        m_firstVisibleIndex = -1;
        m_lastVisibleIndex = -1;
        m_visibleIndexesDirty = false;
        return;
    }

    const int maxIndex = m_model->count() - 1;

    // Calculate the first visible index that is fully visible
    int min = 0;
    int max = maxIndex;
    int mid = 0;
    do {
        mid = (min + max) / 2;
        if (m_itemInfos[mid].rect.top() < m_scrollOffset) {
            min = mid + 1;
        } else {
            max = mid - 1;
        }
    } while (min <= max);

    if (mid > 0) {
        // Include the row before the first fully visible index, as it might
        // be partly visible
        if (m_itemInfos[mid].rect.top() >= m_scrollOffset) {
            --mid;
            Q_ASSERT(m_itemInfos[mid].rect.top() < m_scrollOffset);
        }

        const qreal rowTop = m_itemInfos[mid].rect.top();
        while (mid > 0 && m_itemInfos[mid - 1].rect.top() == rowTop) {
            --mid;
        }
    }
    m_firstVisibleIndex = mid;

    // Calculate the last visible index that is (at least partly) visible
    const int visibleHeight = (m_scrollOrientation == Qt::Horizontal) ? m_size.width() : m_size.height();
    qreal bottom = m_scrollOffset + visibleHeight;
    if (m_model->groupedSorting()) {
        bottom += m_groupHeaderHeight;
    }

    min = m_firstVisibleIndex;
    max = maxIndex;
    do {
        mid = (min + max) / 2;
        if (m_itemInfos[mid].rect.y() <= bottom) {
            min = mid + 1;
        } else {
            max = mid - 1;
        }
    } while (min <= max);

    while (mid > 0 && m_itemInfos[mid].rect.y() > bottom) {
        --mid;
    }
    m_lastVisibleIndex = mid;

    m_visibleIndexesDirty = false;
}

bool KItemListViewLayouter::createGroupHeaders()
{
    if (!m_model->groupedSorting()) {
        return false;
    }

    m_groupItemIndexes.clear();

    const QList<QPair<int, QVariant> > groups = m_model->groups();
    if (groups.isEmpty()) {
        return false;
    }

    for (int i = 0; i < groups.count(); ++i) {
        const int firstItemIndex = groups.at(i).first;
        m_groupItemIndexes.insert(firstItemIndex);
    }

    return true;
}

qreal KItemListViewLayouter::minimumGroupHeaderWidth() const
{
    return 100;
}

#include "kitemlistviewlayouter_p.moc"
