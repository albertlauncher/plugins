// Copyright (C) 2014-2018 Manuel Schneider

#include <QKeyEvent>
#include <QPainter>
#include "actionlist.h"

/** ***************************************************************************/
WidgetBoxModel::ActionList::ActionList(QWidget *parent) : ResizingList(parent) {
    setItemDelegate(new ActionDelegate);
}



/** ***************************************************************************/
bool WidgetBoxModel::ActionList::eventFilter(QObject*, QEvent *event) {

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {

        // Navigation
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        case Qt::Key_Home:
        case Qt::Key_End:
        // Activation
        case Qt::Key_Enter:
        case Qt::Key_Return:
            keyPressEvent(keyEvent);
            return false;
        }
    }
    return false;
}



/** ***************************************************************************/
void WidgetBoxModel::ActionList::ActionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const {

    painter->save();

    QStyleOptionViewItem option = options;
    initStyleOption(&option, index);

    // Draw selection
    option.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

    // Draw text
    painter->setFont(option.font);
    QString text = QFontMetrics(option.font).elidedText(index.data(Qt::DisplayRole).toString(), option.textElideMode, option.rect.width());
    option.widget->style()->drawItemText(painter,
                                         option.rect,
                                         Qt::AlignCenter|Qt::AlignHCenter,
                                         option.palette,
                                         option.state & QStyle::State_Enabled,
                                         text,
                                         (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::WindowText);
    painter->restore();
}
