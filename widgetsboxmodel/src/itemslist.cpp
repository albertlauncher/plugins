// Copyright (c) 2022 Manuel Schneider

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QPixmapCache>
#include <QStylePainter>
#include <QTextDocument>
#include "itemslist.h"
#include "albert.h"


ItemsList::ItemsList(QWidget *parent) : ResizingList(parent)
{
    setItemDelegate(delegate_ = new ItemDelegate(this));

    // Single click activation (segfaults without queued connection)
    connect(this, &ItemsList::clicked, this, &ItemsList::activated, Qt::QueuedConnection);
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    painter->save();
    QStyleOptionViewItem option = options;
    initStyleOption(&option, index);
    QStyle *style = option.widget->style();


    /*
     * fm(x) := fontmetrics of x
     * DR := DisplayRole
     * TR := ToolTipRole
     *  +---------------------+----------------------------------------+
     *  |                     |                                        |
     *  |   +-------------+   |                                        |
     *  |   |             |   |                                        |
     *  |   |             |   |a*fm(DR)/(fm(DR)+fm(TR))    DisplayRole |
     * a|   |     icon    |   |                                        |
     *  |   |             |   |                                        |
     *  |   |             |   +----------------------------------------+
     *  |   |             |   |                                        |
     *  |   +-------------+   |a*fm(TR)/(fm(DR)+fm(TR))  ToolTipRole+x |
     *  |                     |                                        |
     * +---------------------------------------------------------------+
     */


    // Avoid ugly dark blue mouseover background
//    option.state.setFlag(QStyle::State_MouseOver, false); //  &= ~QStyle::State_MouseOver;

    // Calculate rects

    QRect iconRect = QRect(
                QPoint((option.rect.height() - option.decorationSize.width())/2 + option.rect.x(),
                       (option.rect.height() - option.decorationSize.height())/2 + option.rect.y()),
                option.decorationSize);
    QFont font1 = option.font;
    QFont font2 = option.font;
    font2.setPixelSize(subtext_size);
    QFontMetrics fontMetrics1 = QFontMetrics(font1);
    QFontMetrics fontMetrics2 = QFontMetrics(font2);
    uint spacing = (option.rect.height() - fontMetrics1.height() - fontMetrics2.height()) / 2.5;
    QRect textRect = QRect(option.rect.left() + option.rect.height(),
                           option.rect.top() + spacing,
                           option.rect.width() - option.rect.height(),
                           fontMetrics1.height());
    QRect subTextRect = QRect(textRect.left(),
                              textRect.bottom() + spacing/2,
                              textRect.width(),
                              fontMetrics2.height());

    // Draw subelements
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);  // background
    painter->drawPixmap(iconRect, option.icon.pixmap(option.decorationSize * option.widget->devicePixelRatioF()));
    painter->setFont(font1);
    style->drawItemText(painter,
                        textRect,
                        option.displayAlignment,
                        option.palette,
                        option.state & QStyle::State_Enabled,
                        fontMetrics1.elidedText(index.data((int)albert::ItemRoles::TextRole).toString(), option.textElideMode, textRect.width()),
                        (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::WindowText);
    painter->setFont(font2);
    style->drawItemText(painter,
                        subTextRect,
                        option.displayAlignment,
                        option.palette,
                        option.state & QStyle::State_Enabled,
                        fontMetrics2.elidedText(index.data((int)albert::ItemRoles::SubTextRole).toString(), option.textElideMode, subTextRect.width()),
                        (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::WindowText);
//    style->drawPrimitive(QStyle::PE_FrameFocusRect, &option, painter, option.widget);


    // Debug
    if (debug){

        // Std paint
        //style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

        QPen debugPen;
        debugPen.setColor(Qt::red);
        debugPen.setStyle(Qt::DashLine);
        debugPen.setDashOffset(5);
        painter->setPen(debugPen);
        painter->drawRect(option.rect);

        // Std subElementRects
        debugPen.setColor(Qt::magenta);
        painter->setPen(debugPen);
        painter->drawRect(style->subElementRect(QStyle::SE_ItemViewItemDecoration, &option, option.widget));
        painter->drawRect(style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &option, option.widget));
        painter->drawRect(style->subElementRect(QStyle::SE_ItemViewItemText, &option, option.widget));
        painter->drawRect(style->subElementRect(QStyle::SE_ItemViewItemFocusRect, &option, option.widget));
        //    painter->drawRect(style->subElementRect(QStyle::SE_FrameContents, &option, option.widget));
        painter->drawRect(style->subElementRect(QStyle::SE_FrameLayoutItem, &option, option.widget));

        // Style rects, dont respect positions
        //debugPen.setColor(Qt::blue);
        //painter->setPen(debugPen);
        //painter->drawRect(style->itemTextRect(fontMetrics1,option.rect,option.displayAlignment,true,text1));
        //painter->drawRect(style->itemTextRect(fontMetrics2,option.rect,option.displayAlignment,true,text2));
        //painter->drawRect(style->itemPixmapRect(option.rect,option.displayAlignment,option.icon.pixmap(option.decorationSize)));

        // own rects
        debugPen.setColor(QColor("#80FF0000"));
        painter->setPen(debugPen);
        painter->drawRect(iconRect);
        painter->drawRect(textRect);
        painter->drawRect(subTextRect);


    }

    painter->restore();
}





//    auto *w = new QWidget;
//    auto *hl = new QHBoxLayout;
//    auto *vl = new QHBoxLayout;
//    auto *il = new QLabel;
//    auto *tl = new QLabel(index.data((int)albert::ItemRoles::TextRole).toString());
//    auto *sl = new QLabel(index.data((int)albert::ItemRoles::SubTextRole).toString());

//    il->setPixmap(index.data(static_cast<int>(albert::ItemRoles::IconRole)).value<QIcon>()
//                      .pixmap(option.decorationSize.width(), option.decorationSize.width()));

//    vl->addWidget(tl);
//    vl->addWidget(sl);


//    hl->addWidget(il);
//    hl->addLayout(vl);

//    w->setGeometry(option.rect);

//    w->render(painter, QPoint(), option.rect, QWidget::DrawChildren );


//
//
//int main(int argc, char **argv)
//{
//    QApplication app(argc, argv);
//    QWidget panel;
//    QVBoxLayout *l = new QVBoxLayout(&panel);
//    QFrame *viewport = new QFrame;
//    viewport->setFrameShape(QFrame::Box);
//    viewport->setFixedSize(400,600);
//
//    l->addWidget(viewport);
//    QPushButton *b = new QPushButton("Swap");
//    l->addWidget(b);
//    QStateMachine machine;
//    QState *s1 = new QState;
//    QState *s2 = new QState;
//
//    QWidget *w1 = new QCalendarWidget(viewport);
//    w1->setFixedSize(300,500);
//    QWidget *w2 = new QListView(viewport);
//    w2->setFixedSize(300,500);
//
//    QGraphicsBlurEffect *e1 = new QGraphicsBlurEffect(w1);
//    QGraphicsBlurEffect *e2 = new QGraphicsBlurEffect(w2);
//    w1->setGraphicsEffect(e1);
//    w2->setGraphicsEffect(e2);
//
//    s1->assignProperty(w1, "pos", QPoint(50,50));
//    s1->assignProperty(w2, "pos", QPoint(450,50));
//    s1->assignProperty(e1, "blurRadius", 0);
//    s1->assignProperty(e2, "blurRadius", 15);
//    s2->assignProperty(w1, "pos", QPoint(-350, 50));
//    s2->assignProperty(w2, "pos", QPoint(50,50));
//    s2->assignProperty(e1, "blurRadius", 15);
//    s2->assignProperty(e2, "blurRadius", 0);
//
//    s1->addTransition(b, SIGNAL(clicked()), s2);
//    s2->addTransition(b, SIGNAL(clicked()), s1);
//
//    machine.addState(s1);
//    machine.addState(s2);
//
//    QPropertyAnimation *anim1 = new QPropertyAnimation(w1, "pos");
//    QPropertyAnimation *anim2 = new QPropertyAnimation(w2, "pos");
//    anim1->setEasingCurve(QEasingCurve::InOutCubic);
//    anim2->setEasingCurve(anim1->easingCurve());
//    anim1->setDuration(2000);
//    anim2->setDuration(anim1->duration());
//    machine.addDefaultAnimation(anim1);
//    machine.addDefaultAnimation(anim2);
//
//    anim1 = new QPropertyAnimation(e1, "blurRadius");
//    anim2 = new QPropertyAnimation(e2, "blurRadius");
//    anim1->setDuration(1000);
//    anim2->setDuration(anim1->duration());
//    machine.addDefaultAnimation(anim1);
//    machine.addDefaultAnimation(anim2);
//    machine.setInitialState(s1);
//    machine.start();
//    panel.show();
//    return app.exec();
//}