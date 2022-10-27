// Copyright (c) 2022 Manuel Schneider

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QPixmapCache>
#include <QTextDocument>
#include "itemslist.h"
#include "albert.h"

class ItemsList::ItemDelegate final : public QStyledItemDelegate
{
public:
    ItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;

    bool drawIcon = true;
    albert::ItemRoles subTextRole = albert::ItemRoles::SubTextRole;
};

ItemsList::ItemsList(QWidget *parent) : ResizingList(parent)
{
    setItemDelegate(delegate_ = new ItemDelegate(this));

    // Single click activation (segfaults without queued connection)
    connect(this, &ItemsList::clicked, this, &ItemsList::activated, Qt::QueuedConnection);
}

bool ItemsList::displayIcons() const
{
    return delegate_->drawIcon;
}

void ItemsList::setDisplayIcons(bool value)
{
    delegate_->drawIcon = value;
    update();
}

void ItemsList::ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
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

    // Draw icon
    if (drawIcon) {
        QRect iconRect = QRect(
                QPoint((option.rect.height() - option.decorationSize.width())/2 + option.rect.x(),
                       (option.rect.height() - option.decorationSize.height())/2 + option.rect.y()),
                option.decorationSize);


        painter->drawPixmap(iconRect, index.data(static_cast<int>(albert::ItemRoles::IconRole)).value<QIcon>()
                .pixmap(option.decorationSize * option.widget->devicePixelRatioF()));
    }

    // Calculate content rects
    QFont font1 = option.font;
    QFont font2 = option.font;
    font2.setPixelSize(12);
    QFontMetrics fontMetrics1 = QFontMetrics(font1);
    QFontMetrics fontMetrics2 = QFontMetrics(font2);
    QRect contentRect = option.rect;
    contentRect.setLeft(drawIcon ? option.rect.height() : 0);
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
    text = fontMetrics2.elidedText(index.data((int)(option.state.testFlag(QStyle::State_Selected)
                                              ? subTextRole
                                              : albert::ItemRoles::SubTextRole)).toString(),
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