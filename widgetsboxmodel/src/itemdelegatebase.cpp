// Copyright (c) 2014-2024 Manuel Schneider

#include "itemdelegatebase.h"
#include "primitives.h"
#include <QPainter>
#include <QPixmapCache>
using namespace std;

void ItemDelegateBase::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &) const
{
    if(opt.state.testFlag(QStyle::State_Selected))
    {
        QPixmap pm;
        if (const auto cache_key = QString("_ItemViewSelection_%1x%2")
                                       .arg(opt.rect.width()).arg(opt.rect.height());
            !QPixmapCache::find(cache_key, &pm))
        {
            auto dpr = opt.widget->devicePixelRatioF();
            pm = pixelPerfectRoundedRect(opt.rect.size() * dpr,
                                         selection_background_brush,
                                         (int)(selection_border_radius * dpr),
                                         selection_border_brush,
                                         (int)(selection_border_width * dpr));
            pm.setDevicePixelRatio(dpr);
            QPixmapCache::insert(cache_key, pm);
        }
        p->drawPixmap(opt.rect, pm);
    }
}
