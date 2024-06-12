// Copyright (c) 2014-2024 Manuel Schneider

#include "itemdelegatebase.h"
#include "primitives.h"
#include "resultslist.h"
#include "style.h"
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPixmapCache>
#include <QTextCursor>
#include <QTextDocument>
#include <albert/frontend.h>
#include <albert/iconprovider.h>
#include <albert/logging.h>
using namespace albert;
using namespace std;

class ResultsList::ResultDelegate : public ItemDelegateBase
{
public:
    ResultDelegate(ResultsList &l) : ItemDelegateBase(&l), results_list(l) {}
    ResultsList &results_list;

private:
    QSize sizeHint(const QStyleOptionViewItem &o, const QModelIndex&) const override
    {
        auto width = o.widget->width();
        auto height = std::max(style_->result_item_icon_size,
                               results_list.text_font_metrics_.height()
                                   + results_list.subtext_font_metrics_.height()
                                   + style_->result_item_vertical_spacing)
                      + 2 * style_->item_view_item_padding;
        return {width, height};
    }

    void paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const override
    {
        // DEBG << "Painting item";

        // Model data

        const auto text = i.data((int)albert::ItemRoles::TextRole).toString();
        const auto subtext = i.data((int)albert::ItemRoles::SubTextRole).toString();
        const auto icon_urls = i.data((int)albert::ItemRoles::IconUrlsRole).value<QStringList>();

        //
        // LAYOUT
        //


        const QRect icon_rect{
            style_->item_view_item_padding,
            o.rect.y() + (o.rect.height() - style_->result_item_icon_size)/2,
            style_->result_item_icon_size,
            style_->result_item_icon_size
        };

        const auto texts_x = style_->item_view_item_padding
                             + style_->result_item_icon_size
                             + style_->result_item_horizontal_spacing;
        const auto texts_width = o.rect.width() - texts_x - style_->item_view_item_padding;
        const auto texts_height = results_list.text_font_metrics_.height()
                                  + results_list.subtext_font_metrics_.height()
                                  + style_->result_item_vertical_spacing;
        // auto texts_y = (content_rect.height() - texts_height)/2,

        const QRect text_rect{
            texts_x,
            o.rect.y() + (o.rect.height() - texts_height)/2,
            texts_width,
            results_list.text_font_metrics_.height()
        };

        const QRect subtext_rect{
            texts_x,
            o.rect.y()
                + (o.rect.height() - texts_height)/2
                + results_list.text_font_metrics_.height()
                + style_->result_item_vertical_spacing,
            texts_width,
            results_list.subtext_font_metrics_.height()
        };

        //
        // PAINT
        //

        p->save();

        // Draw selection
        ItemDelegateBase::paint(p, o, i); // background
        if(o.state.testFlag(QStyle::State_Selected))
        {
            QPen pen(o.widget->palette().highlightedText(), 2);
            p->setPen(pen);
        }

        // Draw icon
        QPixmap pm;
        const auto icon_cache_key = QString("albert$%1%2x%3")
                                        .arg(icon_urls.join(""))
                                        .arg(style_->result_item_icon_size)
                                        .arg(o.widget->devicePixelRatioF());
        if (!QPixmapCache::find(icon_cache_key, &pm))
        {
            pm = pixmapFromUrls(icon_urls,
                                QSize(style_->result_item_icon_size, style_->result_item_icon_size)
                                    * o.widget->devicePixelRatioF());  // yes, needed
            // QPixmapCache::insert(icon_cache_key, pm);
        }
        p->drawPixmap(icon_rect, pm);

        // Draw text
        p->setFont(results_list.text_font_);
        p->setPen(QPen(style_->result_item_text_color, 0));
        const QString elided_text = results_list.text_font_metrics_.elidedText(text, o.textElideMode, text_rect.width());
        p->drawText(text_rect.bottomLeft()-QPoint(0,results_list.text_font_metrics_.descent()-1), elided_text);  // Adjusted by descent sice origin seems to be the baseline
        // p->drawText(text_rect, elided_text);  // Clips, here for checks only

        // Draw subtext
        p->setFont(results_list.subtext_font_);
        p->setPen(QPen(style_->result_item_subtext_color, 0));
        const QString elided_subtext = results_list.subtext_font_metrics_.elidedText(subtext, o.textElideMode, subtext_rect.width());
        p->drawText(subtext_rect.bottomLeft()-QPoint(0,results_list.subtext_font_metrics_.descent()-1), elided_subtext);
        // p->drawText(subtext_rect, elided_subtext);  // Clips, here for checks only

        if (style_->draw_debug_overlays){

            const QRect contents_rect = o.rect.adjusted(
                style_->item_view_item_padding,
                style_->item_view_item_padding,
                -style_->item_view_item_padding,
                -style_->item_view_item_padding
                );

            drawDebugRect(*p, o.rect, "ResultDelegate");
            drawDebugRect(*p, contents_rect, "contents_rect");
            drawDebugRect(*p, icon_rect, "icon_rect");
            drawDebugRect(*p, text_rect, "text_rect");
            drawDebugRect(*p, subtext_rect, "subtext_rect");
        }

        p->restore();
    }
};


ResultsList::ResultsList(QWidget *parent):
    ResizingList(parent),
    delegate(new ResultDelegate(*this)),
    text_font_(font()),
    subtext_font_(font()),
    text_font_metrics_(text_font_),
    subtext_font_metrics_(subtext_font_)
{
    setItemDelegate(delegate);
}

void ResultsList::setStyle(const Style *s)
{
    ResizingList::setStyle(s);

    delegate->setStyle(s);

    text_font_.setPointSize(style->result_item_text_font_size);
    text_font_metrics_ = QFontMetrics(text_font_);

    subtext_font_.setPointSize(style->result_item_subtext_font_size);
    subtext_font_metrics_ = QFontMetrics(subtext_font_);
}


















/* Archive and useful

# QCommonStyle::drawControl

    case CE_ItemViewItem:
        if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
            p->save();
            // the style calling this might want to clip, so respect any region already set
            const QRegion clipRegion = p->hasClipping() ? (p->clipRegion() & opt->rect) : opt->rect;
            p->setClipRegion(clipRegion);

            QRect checkRect = proxy()->subElementRect(SE_ItemViewItemCheckIndicator, vopt, widget);
            QRect iconRect = proxy()->subElementRect(SE_ItemViewItemDecoration, vopt, widget);
            QRect textRect = proxy()->subElementRect(SE_ItemViewItemText, vopt, widget);

            // draw the background
            proxy()->drawPrimitive(PE_PanelItemViewItem, opt, p, widget);

            // draw the check mark
            if (vopt->features & QStyleOptionViewItem::HasCheckIndicator) {
                QStyleOptionViewItem option(*vopt);
                option.rect = checkRect;
                option.state = option.state & ~QStyle::State_HasFocus;

                switch (vopt->checkState) {
                case Qt::Unchecked:
                    option.state |= QStyle::State_Off;
                    break;
                case Qt::PartiallyChecked:
                    option.state |= QStyle::State_NoChange;
                    break;
                case Qt::Checked:
                    option.state |= QStyle::State_On;
                    break;
                }
                proxy()->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &option, p, widget);
            }

            // draw the icon
            QIcon::Mode mode = QIcon::Normal;
            if (!(vopt->state & QStyle::State_Enabled))
                mode = QIcon::Disabled;
            else if (vopt->state & QStyle::State_Selected)
                mode = QIcon::Selected;
            QIcon::State state = vopt->state & QStyle::State_Open ? QIcon::On : QIcon::Off;
            vopt->icon.paint(p, iconRect, vopt->decorationAlignment, mode, state);

            // draw the text
            if (!vopt->text.isEmpty()) {
                QPalette::ColorGroup cg = vopt->state & QStyle::State_Enabled
                                      ? QPalette::Normal : QPalette::Disabled;
                if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                    cg = QPalette::Inactive;

                if (vopt->state & QStyle::State_Selected) {
                    p->setPen(vopt->palette.color(cg, QPalette::HighlightedText));
                } else {
                    p->setPen(vopt->palette.color(cg, QPalette::Text));
                }
                if (vopt->state & QStyle::State_Editing) {
                    p->setPen(vopt->palette.color(cg, QPalette::Text));
                    p->drawRect(textRect.adjusted(0, 0, -1, -1));
                }

                d->viewItemDrawText(p, vopt, textRect);
            }

            // draw the focus rect
             if (vopt->state & QStyle::State_HasFocus) {
                QStyleOptionFocusRect o;
                o.QStyleOption::operator=(*vopt);
                o.rect = proxy()->subElementRect(SE_ItemViewItemFocusRect, vopt, widget);
                o.state |= QStyle::State_KeyboardFocusChange;
                o.state |= QStyle::State_Item;
                QPalette::ColorGroup cg = (vopt->state & QStyle::State_Enabled)
                              ? QPalette::Normal : QPalette::Disabled;
                o.backgroundColor = vopt->palette.color(cg, (vopt->state & QStyle::State_Selected)
                                             ? QPalette::Highlight : QPalette::Window);
                proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, widget);
            }

             p->restore();
        }
        break;



    case SE_ItemViewItemCheckIndicator:
        if (!qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
            r = subElementRect(SE_CheckBoxIndicator, opt, widget);
            break;
        }
        Q_FALLTHROUGH();
    case SE_ItemViewItemDecoration:
    case SE_ItemViewItemText:
    case SE_ItemViewItemFocusRect:
        if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {
            if (!d->isViewItemCached(*vopt)) {
                d->viewItemLayout(vopt, &d->checkRect, &d->decorationRect, &d->displayRect, false);
                if (d->cachedOption) {
                    delete d->cachedOption;
                    d->cachedOption = nullptr;
                }
                d->cachedOption = new QStyleOptionViewItem(*vopt);
            }
            if (sr == SE_ItemViewItemCheckIndicator)
                r = d->checkRect;
            else if (sr == SE_ItemViewItemDecoration)
                r = d->decorationRect;
            else if (sr == SE_ItemViewItemText || sr == SE_ItemViewItemFocusRect)
                r = d->displayRect;
                               }
        break;


































int main(int argc, char **argv)
{
   QApplication app(argc, argv);
   QWidget panel;
   QVBoxLayout *l = new QVBoxLayout(&panel);
   QFrame *viewport = new QFrame;
   viewport->setFrameShape(QFrame::Box);
   viewport->setFixedSize(400,600);

   l->addWidget(viewport);
   QPushButton *b = new QPushButton("Swap");
   l->addWidget(b);
   QStateMachine machine;
   QState *s1 = new QState;
   QState *s2 = new QState;

   QWidget *w1 = new QCalendarWidget(viewport);
   w1->setFixedSize(300,500);
   QWidget *w2 = new QListView(viewport);
   w2->setFixedSize(300,500);

   QGraphicsBlurEffect *e1 = new QGraphicsBlurEffect(w1);
   QGraphicsBlurEffect *e2 = new QGraphicsBlurEffect(w2);
   w1->setGraphicsEffect(e1);
   w2->setGraphicsEffect(e2);

   s1->assignProperty(w1, "pos", QPoint(50,50));
   s1->assignProperty(w2, "pos", QPoint(450,50));
   s1->assignProperty(e1, "blurRadius", 0);
   s1->assignProperty(e2, "blurRadius", 15);
   s2->assignProperty(w1, "pos", QPoint(-350, 50));
   s2->assignProperty(w2, "pos", QPoint(50,50));
   s2->assignProperty(e1, "blurRadius", 15);
   s2->assignProperty(e2, "blurRadius", 0);

   s1->addTransition(b, SIGNAL(clicked()), s2);
   s2->addTransition(b, SIGNAL(clicked()), s1);

   machine.addState(s1);
   machine.addState(s2);

   QPropertyAnimation *anim1 = new QPropertyAnimation(w1, "pos");
   QPropertyAnimation *anim2 = new QPropertyAnimation(w2, "pos");
   anim1->setEasingCurve(QEasingCurve::InOutCubic);
   anim2->setEasingCurve(anim1->easingCurve());
   anim1->setDuration(2000);
   anim2->setDuration(anim1->duration());
   machine.addDefaultAnimation(anim1);
   machine.addDefaultAnimation(anim2);

   anim1 = new QPropertyAnimation(e1, "blurRadius");
   anim2 = new QPropertyAnimation(e2, "blurRadius");
   anim1->setDuration(1000);
   anim2->setDuration(anim1->duration());
   machine.addDefaultAnimation(anim1);
   machine.addDefaultAnimation(anim2);
   machine.setInitialState(s1);
   machine.start();
   panel.show();
   return app.exec();
}

*/
