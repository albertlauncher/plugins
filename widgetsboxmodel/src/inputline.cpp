// Copyright (c) 2022 Manuel Schneider

#include "inputline.h"
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOptionFrame>

InputLine::InputLine(QWidget *parent) : QLineEdit(parent)
{
}

void InputLine::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);
    if (!hasFocus())
        return;

    if (!input_hint.isNull()) {
        QString hint;
        if (input_hint.startsWith(text()))
            hint = input_hint.mid(text().length());
        else
            hint = QString(" %1").arg(input_hint);

        QStyleOptionFrame panel;
        initStyleOption(&panel);
        QPainter p(this);
        ensurePolished(); // ensure font() is up to date

        QRect content_rect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
        content_rect = content_rect.marginsRemoved(textMargins());
        content_rect.adjust(2,1,-2,-1); // https://codebrowser.dev/qt5/qtbase/src/widgets/widgets/qlineedit_p.cpp.html#QLineEditPrivate::verticalMargin

        auto fm = fontMetrics();
        content_rect.adjust(fm.horizontalAdvance(text()), 0, 0, 0);
        auto text = fm.elidedText(hint, Qt::ElideRight, content_rect.width());
        auto color = panel.palette.placeholderText().color();
        color.setAlpha(80);
        p.setPen(color);
        p.drawText(content_rect, Qt::TextSingleLine, text);
    }
}

void InputLine::setInputHint(const QString &text)
{
    input_hint = text;
    setToolTip(text);
    update();
}
