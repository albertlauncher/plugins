// Copyright (c) 2014-2025 Manuel Schneider

#include "primitives.h"
#include "resultitemmodel.h"
#include "resultslist.h"
#include <QApplication>
#include <QPainter>
#include <QPixmapCache>
#include <albert/iconprovider.h>
using namespace albert;
using namespace std;


class ResultsListDelegate : public ItemDelegateBase
{
public:
    ResultsListDelegate();

    QFont subtext_font;
    QColor subtext_color;
    QColor selection_subtext_color;
    QFontMetrics subtext_font_metrics;

    int icon_size;

    int horizontal_spacing;
    int vertical_spacing;

    bool draw_debug_overlays;

    QSize sizeHint(const QStyleOptionViewItem &o, const QModelIndex&) const override;
    void paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const override;

};

//--------------------------------------------------------------------------------------------------

ResultsList::ResultsList(QWidget *parent):
    ResizingList(parent)
{
    delegate_ = new ResultsListDelegate;
    setItemDelegate(delegate_);
}

ResultsList::~ResultsList() { delete delegate_; }

ItemDelegateBase *ResultsList::delegate() const { return delegate_; }

uint ResultsList::subtextFontSize() const { return delegate_->subtext_font.pointSize(); }

void ResultsList::setSubextFontSize(uint v)
{
    delegate_->subtext_font.setPointSize(v);
    delegate_->subtext_font_metrics = QFontMetrics(delegate_->subtext_font);
    relayout();
}

QColor ResultsList::subtextColor() const { return delegate_->subtext_color; }

void ResultsList::setSubtextColor(QColor v) { delegate_->subtext_color = v; update(); }

QColor ResultsList::selectionSubtextColor() const { return delegate_->selection_subtext_color; }

void ResultsList::setSelectionSubextColor(QColor v) { delegate_->selection_subtext_color = v; update(); }

uint ResultsList::horizonzalSpacing() const { return delegate_->horizontal_spacing; }

void ResultsList::setHorizonzalSpacing(uint v) { delegate_->horizontal_spacing = v; relayout(); }

uint ResultsList::verticalSpacing() const { return delegate_->vertical_spacing; }

void ResultsList::setVerticalSpacing(uint v) { delegate_->vertical_spacing = v; relayout(); }

uint ResultsList::iconSize() const { return delegate_->icon_size; }

void ResultsList::setIconSite(uint v) { delegate_->icon_size = v; relayout(); }

//--------------------------------------------------------------------------------------------------

ResultsListDelegate::ResultsListDelegate():
    subtext_font(QApplication::font()),
    subtext_font_metrics(subtext_font)
{

}

QSize ResultsListDelegate::sizeHint(const QStyleOptionViewItem &o, const QModelIndex &) const
{
    auto width = o.widget->width();
    auto height = std::max(icon_size,
                           text_font_metrics.height()
                               + subtext_font_metrics.height()
                               + vertical_spacing)
                  + 2 * padding;
    return {width, height};
}

void ResultsListDelegate::paint(QPainter *p,
                                const QStyleOptionViewItem &o,
                                const QModelIndex &i) const
{
    //
    // LAYOUT
    //

    const QRect icon_rect{padding,
                          o.rect.y() + (o.rect.height() - icon_size) / 2,
                          icon_size,
                          icon_size};

    const auto texts_x = padding + icon_size + horizontal_spacing;

    const auto texts_width = o.rect.width() - texts_x - padding;

    const auto texts_height = text_font_metrics.height()
                              + subtext_font_metrics.height()
                              + vertical_spacing;

    const QRect text_rect{texts_x,
                          o.rect.y() + (o.rect.height() - texts_height) / 2,
                          texts_width,
                          text_font_metrics.height()};

    const QRect subtext_rect{texts_x,
                             o.rect.y() + (o.rect.height() - texts_height) / 2
                                 + text_font_metrics.height()
                                 + vertical_spacing,
                             texts_width,
                             subtext_font_metrics.height()};

    //
    // DATA
    //

    const auto text = text_font_metrics.elidedText(i.data((int) ItemRoles::TextRole).toString(),
                                                   o.textElideMode,
                                                   text_rect.width());

    const auto subtext = subtext_font_metrics.elidedText(i.data((int) ItemRoles::SubTextRole).toString(),
                                                         o.textElideMode,
                                                         subtext_rect.width());

    const auto icon_urls = i.data((int) ItemRoles::IconUrlsRole).value<QStringList>();
    const auto dpr = o.widget->devicePixelRatioF();
    auto selected = o.state.testFlag(QStyle::State_Selected);

    QPixmap pm;
    const auto cache_key = QString("$%1%2result_icon").arg(icon_size * dpr).arg(icon_urls.join(""));
    if (!QPixmapCache::find(cache_key, &pm))
    {
        pm = pixmapFromUrls(icon_urls, QSize(icon_size, icon_size) * dpr);
        pm.setDevicePixelRatio(dpr);
        QPixmapCache::insert(cache_key, pm);
    }

    //
    // PAINT
    //

    p->save();

    // Draw selection
    ItemDelegateBase::paint(p, o, i);

    // Draw icon (such that it is centered in the icon_rect)
    p->drawPixmap(
        icon_rect.x() + (icon_rect.width() - (int)pm.deviceIndependentSize().width()) / 2,
        icon_rect.y() + (icon_rect.height() - (int)pm.deviceIndependentSize().height()) / 2,
        pm);

    // Draw text
    p->setFont(text_font);
    p->setPen(QPen(selected ? selection_text_color : text_color, 0));
    // Clips. Adjust by descent sice origin seems to be the baseline
    // p->drawText(text_rect, text);
    p->drawText(text_rect.bottomLeft() - QPoint(0, text_font_metrics.descent() - 1), text);

    // Draw subtext
    p->setFont(subtext_font);
    p->setPen(QPen(selected ? selection_subtext_color : subtext_color, 0));
    // Clips. Adjust by descent sice origin seems to be the baseline
    // p->drawText(subtext_rect, subtext);
    p->drawText(subtext_rect.bottomLeft() - QPoint(0, subtext_font_metrics.descent() - 1), subtext);

    if (draw_debug_overlays)
    {
        drawDebugRect(*p, o.rect, "ResultDelegate");
        drawDebugRect(*p, icon_rect, "icon_rect");
        drawDebugRect(*p, text_rect, "text_rect");
        drawDebugRect(*p, subtext_rect, "subtext_rect");
    }

    p->restore();
}
