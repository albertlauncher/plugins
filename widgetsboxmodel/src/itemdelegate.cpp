// Copyright (c) 2014-2024 Manuel Schneider

#include "itemdelegate.h"
#include <QPainter>
#include <QPixmapCache>
#include <albert/frontend.h>
#include <albert/iconprovider.h>
using namespace albert;


ItemDelegate::ItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    painter->save();

    QStyleOptionViewItem option = options;
    initStyleOption(&option, index);

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
    option.state.setFlag(QStyle::State_MouseOver, false); //  &= ~QStyle::State_MouseOver;

    // Draw selection
    option.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

    // Compute icon rect
    QRect icon_rect = QRect(
        QPoint((option.rect.height() - option.decorationSize.width())/2 + option.rect.x(),
               (option.rect.height() - option.decorationSize.height())/2 + option.rect.y()),
        option.decorationSize);

    // Get the icon
    QPixmap pm;
    auto icon_urls = index.data(static_cast<int>(albert::ItemRoles::IconUrlsRole)).value<QStringList>();
    auto icon_size = option.decorationSize.height();
    const auto icon_cache_key = QString("albert$%1%2x%3")
                                    .arg(icon_urls.join(""))
                                    .arg(icon_size)
                                    .arg(option.widget->devicePixelRatioF());
    if (!QPixmapCache::find(icon_cache_key, &pm))
    {
        pm = pixmapFromUrls(icon_urls,
                            QSize(icon_size, icon_size) * option.widget->devicePixelRatioF());
        pm.setDevicePixelRatio(option.widget->devicePixelRatioF());
        QPixmapCache::insert(icon_cache_key, pm);
    }

    // Draw the icon such that it is centered in the icon_rect
    painter->drawPixmap(icon_rect.x()
                            + (icon_rect.width() - (int)pm.deviceIndependentSize().width()) / 2,
                        icon_rect.y()
                            + (icon_rect.height() - (int)pm.deviceIndependentSize().height()) / 2,
                        pm);

    // Calculate content rects
    QFont font1 = option.font;
    QFont font2 = option.font;
    font2.setPixelSize(12);
    QFontMetrics fontMetrics1 = QFontMetrics(font1);
    QFontMetrics fontMetrics2 = QFontMetrics(font2);
    QRect contentRect = option.rect;
    contentRect.setLeft(option.rect.height());
    contentRect.setTop(option.rect.y()+option.rect.height()/2-(fontMetrics1.height()+fontMetrics2.height())/2);
    contentRect.setBottom(option.rect.y()+option.rect.height()/2+(fontMetrics1.height()+fontMetrics2.height())/2);
    QRect textRect = contentRect.adjusted(0,-2,0,-fontMetrics2.height()-2);
    QRect subTextRect = contentRect.adjusted(0,fontMetrics1.height()-2,0,-2);

    // Draw item text
    QString text = fontMetrics1.elidedText(index.data((int)albert::ItemRoles::TextRole).toString(),
                                           option.textElideMode, textRect.width());
    painter->setFont(font1);
    option.widget->style()->drawItemText(painter,
                                         textRect,
                                         option.displayAlignment,
                                         option.palette,
                                         option.state & QStyle::State_Enabled,
                                         text,
                                         (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::WindowText);

    // Draw item subtext
    text = fontMetrics2.elidedText(index.data((int)albert::ItemRoles::SubTextRole).toString(),
                                   option.textElideMode, subTextRect.width());
    painter->setFont(font2);
    option.widget->style()->drawItemText(painter,
                                         subTextRect,
                                         Qt::AlignBottom|Qt::AlignLeft,
                                         option.palette,
                                         option.state & QStyle::State_Enabled,
                                         text,
                                         (option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::WindowText);


    //    // Test
    //    painter->drawRect(option.rect);
    //    painter->setPen(Qt::red);
    //    painter->drawRect(contentRect);
    //    painter->setPen(Qt::blue);
    //    painter->drawRect(textRect);
    //    painter->setPen(Qt::green);
    //    painter->drawRect(subTextRect);
    //    painter->fillRect(option.rect, Qt::magenta);
    //    painter->fillRect(contentRect, Qt::red);
    //    painter->fillRect(textRect, Qt::blue);
    //    painter->fillRect(subTextRect, Qt::yellow);

    // Debug
//    if (true){
//        // Std paint
//        //style->drawControl(QStyle::CE_ItemViewItem, &option, painter, option.widget);

//        QPen debugPen;
//        debugPen.setColor(Qt::red);
//        debugPen.setStyle(Qt::DashLine);
//        debugPen.setDashOffset(5);
//        painter->setPen(debugPen);
//        painter->drawRect(option.rect);

//        // Std subElementRects
////        debugPen.setColor(Qt::magenta);
////        painter->setPen(debugPen);
////        painter->drawRect(option.widget->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &option, option.widget));
////        painter->drawRect(option.widget->style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &option, option.widget));
////        painter->drawRect(option.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &option, option.widget));
////        painter->drawRect(option.widget->style()->subElementRect(QStyle::SE_ItemViewItemFocusRect, &option, option.widget));
////        //    painter->drawRect(style->subElementRect(QStyle::SE_FrameContents, &option, option.widget));
////        painter->drawRect(option.widget->style()->subElementRect(QStyle::SE_FrameLayoutItem, &option, option.widget));

//        // Style rects, dont respect positions
//        //debugPen.setColor(Qt::blue);
//        //painter->setPen(debugPen);
//        //painter->drawRect(style->itemTextRect(fontMetrics1,option.rect,option.displayAlignment,true,text1));
//        //painter->drawRect(style->itemTextRect(fontMetrics2,option.rect,option.displayAlignment,true,text2));
//        //painter->drawRect(style->itemPixmapRect(option.rect,option.displayAlignment,option.icon.pixmap(option.decor>

//        // own rects
//        debugPen.setColor(QColor("#80FF0000"));
//        painter->setPen(debugPen);
//        painter->drawRect(iconRect);
//        painter->drawRect(textRect);
//        painter->drawRect(subTextRect);
//    }


    painter->restore();
}
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
