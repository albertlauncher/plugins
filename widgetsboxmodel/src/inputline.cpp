// Copyright (c) 2022-2024 Manuel Schneider

#include "inputline.h"
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOptionFrame>

InputLine::InputLine(QWidget *parent) : QLineEdit(parent)
{
    connect(this, &QLineEdit::textEdited, this, [this]{
        history_.resetIterator();
        user_text_ = text();
    });

    // Clear input hint on text change
    connect(this, &QLineEdit::textChanged, this,
            [this](){ input_hint_ = QString(); });
}

void InputLine::setInputHint(const QString &text)
{
    input_hint_ = text;
    setToolTip(text);
    update();
}

void InputLine::setTriggerLength(uint len) { trigger_length_ = len; }

void InputLine::next(bool search)
{
    auto t = history_.next(search ? user_text_ : QString());

    // Without ClearOnHide the text is already in the input
    // I.e. the first item in history equals the input text
    if (t == text())
        t = history_.next(search ? user_text_ : QString());

    // Empty text is history sentinel
    if (!t.isEmpty())
        setText(t);
}

void InputLine::previous(bool search)
{
    auto t = history_.prev(search ? user_text_ : QString());

    // Empty text is history sentinel, restore user text
    if (t.isEmpty())
        setText(user_text_);
    else
        setText(t);
}

void InputLine::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);

    QStyleOptionFrame panel;
    initStyleOption(&panel);

    QRect content_rect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
    content_rect = content_rect.marginsRemoved(textMargins());
    content_rect.adjust(2,1,-2,-1); // https://codebrowser.dev/qt5/qtbase/src/widgets/widgets/qlineedit_p.cpp.html#QLineEditPrivate::verticalMargin

    QPainter p(this);
    p.setPen(panel.palette.placeholderText().color());

    if (!input_hint_.isNull())
    {
        QString hint;
        if (input_hint_.startsWith(text()))
            hint = input_hint_.mid(text().length());
        else
            hint = QString(" %1").arg(input_hint_);

        auto fm = fontMetrics();
        auto r = content_rect;
        r.adjust(fm.horizontalAdvance(text()), 0, 0, 0);
        auto t = fm.elidedText(hint, Qt::ElideRight, r.width());

        p.drawText(r, Qt::TextSingleLine, t);
    }

    // TODO glitches when line is overflowing, need something better than QLineEdit, maybe QTextEdit
    // if (trigger_length_)
    // {
    //     auto f = p.font();
    //     f.setUnderline(true);
    //     p.setFont(f);
    //     p.drawText(content_rect, Qt::TextSingleLine, text().left(trigger_length_));
    // }
}

void InputLine::hideEvent(QHideEvent *event)
{
    history_.add(text());
    history_.resetIterator();
    user_text_.clear();

    if (clear_on_hide)
        clear();
    else
        selectAll();

    QLineEdit::hideEvent(event);
}
