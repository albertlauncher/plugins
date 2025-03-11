// Copyright (c) 2014-2025 Manuel Schneider

#include "itemdelegatebase.h"
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

    QFont text_font;
    QFont subtext_font;

    QColor text_color;
    QColor subtext_color;

    QFontMetrics text_font_metrics;
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

ResultsList::~ResultsList()
{
    delete delegate_;
}

ItemDelegateBase *ResultsList::delegate() const { return delegate_; }

uint ResultsList::textFontSize() const { return delegate_->text_font.pointSize(); }

void ResultsList::setTextFontSize(uint val)
{
    delegate_->text_font.setPointSize(val);
    delegate_->text_font_metrics = QFontMetrics(delegate_->text_font);
    relayout();
}

uint ResultsList::subtextFontSize() const { return delegate_->subtext_font.pointSize(); }

void ResultsList::setSubextFontSize(uint val)
{
    delegate_->subtext_font.setPointSize(val);
    delegate_->subtext_font_metrics = QFontMetrics(delegate_->subtext_font);
    relayout();
}

QColor ResultsList::textColor() const { return delegate_->text_color; }

void ResultsList::setTextColor(QColor val) { delegate_->text_color = val; update(); }

QColor ResultsList::subtextColor() const { return delegate_->subtext_color; }

void ResultsList::setSubextColor(QColor val) { delegate_->subtext_color = val; update(); }

uint ResultsList::horizonzalSpacing() const { return delegate_->horizontal_spacing; }

void ResultsList::setHorizonzalSpacing(uint val) { delegate_->horizontal_spacing = val; relayout(); }

uint ResultsList::verticalSpacing() const { return delegate_->vertical_spacing; }

void ResultsList::setVerticalSpacing(uint val) { delegate_->vertical_spacing = val; relayout(); }

uint ResultsList::iconSize() const { return delegate_->icon_size; }

void ResultsList::setIconSite(uint val) { delegate_->icon_size = val; relayout(); }

bool ResultsList::debugMode() const { return delegate_->draw_debug_overlays; }

void ResultsList::setDebugMode(bool val) { delegate_->draw_debug_overlays = val; update(); }

//--------------------------------------------------------------------------------------------------

ResultsListDelegate::ResultsListDelegate():
    text_font(QApplication::font()),
    subtext_font(QApplication::font()),
    text_font_metrics(text_font),
    subtext_font_metrics(subtext_font),
    draw_debug_overlays(false)
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
    auto highlight_text_color = o.widget->palette().highlightedText();

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
    p->setPen(QPen(selected ? highlight_text_color : text_color, 0));
    // Clips. Adjust by descent sice origin seems to be the baseline
    // p->drawText(text_rect, text);
    p->drawText(text_rect.bottomLeft() - QPoint(0, text_font_metrics.descent() - 1), text);

    // Draw subtext
    p->setFont(subtext_font);
    p->setPen(QPen(selected ? highlight_text_color : subtext_color, 0));
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
