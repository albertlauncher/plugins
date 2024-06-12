// Copyright (c) 2022-2024 Manuel Schneider

#include "inputline.h"
#include "primitives.h"
#include "style.h"
#include <QApplication>
#include <QApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOptionFrame>
#include <albert/logging.h>

InputLine::InputLine(QWidget *parent) : QTextEdit(parent)
{
    document()->setDocumentMargin(1); // 0 would be optimal but clips bearing

    setAcceptRichText(false);
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWordWrapMode(QTextOption::NoWrap);
    viewport()->setAutoFillBackground(false);

    // connect(this, &QLineEdit::textEdited, this, [this]{
    //     history_.resetIterator();
    //     user_text_ = text();
    // });

    // // Clear input hint on text change
    // connect(this, &QTextEdit::textChanged, this, [this]{
    //     // auto c = textCursor();
    //     // c.po
    //     // horizontal_advance_ = fontMetrics().horizontalAdvance(t);
    //     // synopsis_ = {};
    //     // completion_ = {};
    //     // updateTooltip();
    // });

    // auto fixHeight = [this]{
    //     INFO << "INPUTLINE fm lineSpacing" << fontMetrics().lineSpacing();
    //     INFO << "INPUTLINE doc height" << document()->size().height();
    //     INFO << "INPUTLINE doc margin" << document()->documentMargin();
    //     int height = document()->size().height() ;//+ document()->documentMargin();
    //     INFO << "Final height" << height;
    //     setFixedHeight(height);
    // };

    // connect(this, &QTextEdit::textChanged, this, fixHeight);
    // fixHeight();


    connect(document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged,
            this, [this](const QSizeF &newSize){ setFixedHeight(newSize.height()); });

    connect(this, &QTextEdit::textChanged, this, [this]
    {
        CRIT << "textChanged" << toHtml();

        // Save and emit plainTextChanged if necessary
        auto t = toPlainText();
        if (t != plain_text_)
        {
            plain_text_ = t;
            emit plainTextChanged(plain_text_);
        }
    });

    // Why was this necessary???
    // // Fix charFormat when cursor is at trigger boundary
    // connect(this, &QTextEdit::cursorPositionChanged, this, [this]{
    //     auto c = textCursor();
    //     if ((int)trigger_length_ == c.position())
    //     {
    //         c.joinPreviousEditBlock();
    //         setCurrentCharFormat(c.blockCharFormat());
    //         c.endEditBlock();
    //     }
    // });
}

// void InputLine::setSynopsis(const QString &t)
// {
//     synopsis_ = t;
//     updateTooltip();
//     // update();
// }

void InputLine::setCompletion(const QString &t)
{
    completion_ = t.toLower();
    updateTooltip();
    // update();
}

void InputLine::updateTooltip()
{
    QStringList tooltip;
    // if (!synopsis_.isNull())
        // tooltip << synopsis_;
    if (!completion_.isNull())
        tooltip << completion_;
    setToolTip(tooltip.join('\n'));
}

void InputLine::setTriggerLength(uint len)
{
    CRIT << "setTriggerLength" << len;
    trigger_length_ = len;
    colorizeTrigger();
}

void InputLine::colorizeTrigger()
{
    INFO << "colorizeTrigger";

    QTextCharFormat f;
    QTextCursor c{document()};

    c.joinPreviousEditBlock();

    c.setPosition(trigger_length_, QTextCursor::KeepAnchor);
    f.setForeground(palette().placeholderText().color());
    c.setCharFormat(f);

    c.clearSelection();

    c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    f.setForeground(palette().text().color());
    c.setCharFormat(c.blockCharFormat());

    c.endEditBlock();
}

QString InputLine::text() const
{ return plain_text_; }

void InputLine::setText(QString t)
{
    QTextCursor c{document()};
    c.beginEditBlock();
    c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    c.removeSelectedText();
    c.insertText(t);
    c.endEditBlock();
    setTextCursor(c);
}

void InputLine::next()
{
    auto t = history_.next(history_search ? user_text_ : QString());

    // Without ClearOnHide the text is already in the input
    // I.e. the first item in history equals the input text
    if (t == text())
        t = history_.next(history_search ? user_text_ : QString());

    setText(t);
}

void InputLine::previous()
{
    auto t = history_.prev(history_search ? user_text_ : QString());
    if (!t.isEmpty())
        setText(t);
}

void InputLine::setStyle(const Style *s)
{
    style_ = s;

    auto f = font();
    f.setPointSize(s->input_line_font_size);
    setFont(f);

    update();
}

void InputLine::paintEvent(QPaintEvent *event)
{
    // DEBG << "InputLine::paintEvent";
    QTextEdit::paintEvent(event);

    if (style_->draw_debug_overlays){
        QPainter p(viewport());
        drawDebugRect(p, rect(), "InputLine::rect");
        drawDebugRect(p, contentsRect(), "InputLine::contentsRect");
        drawDebugRect(p, viewport()->childrenRect(), "InputLine::childrenRect");
    }



}

void InputLine::showEvent(QShowEvent *event)
{

    // if (!clear_on_hide)
    // {

    //     QTextCursor c{document()};
    //     c.movePosition(QTextCursor::NextCharacter);
    //     c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    //     setTextCursor(c);

    //     // selectAll();
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

    QTextEdit::hideEvent(event);
}

void InputLine::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Tab)
    {
        if (!completion_.isEmpty())
            setText(completion_);
        event->accept();
    }

#if defined Q_OS_MACOS
    else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Backspace)
    {
        QTextCursor c = textCursor();
        c.beginEditBlock();
        c.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
        c.removeSelectedText();
        c.endEditBlock();
    }
#endif

    else
        QTextEdit::keyPressEvent(event);
}

















// QLineEdit::paintEvent(event);
// QStyleOptionFrame o;
// initStyleOption(&o);

//     QRect content_rect = style()->subElementRect(QStyle::SE_LineEditContents, &o, this);
//     content_rect = content_rect.marginsRemoved(textMargins());
//     content_rect.adjust(2,1,-2,-1); // https://codebrowser.dev/qt5/qtbase/src/widgets/widgets/qlineedit_p.cpp.html#QLineEditPrivate::verticalMargin

//     QPainter p(this);
//     p.setPen(o.palette.placeholderText().color());

//     if (!completion_.isNull() && !synopsis_.isNull())
//     {
//         QFont font = p.font();
//         font.setPixelSize(font.pixelSize()/2);
//         p.setFont(font);

//         auto r = content_rect.adjusted(horizontal_advance_ + 5, content_rect.height()/2, 0, 0);
//         p.drawText(r, Qt::TextSingleLine, p.fontMetrics().elidedText(completion_, Qt::ElideMiddle, r.width()));

//         font.setItalic(true);
//         p.setFont(font);

//         r = content_rect.adjusted(horizontal_advance_ + 5, 0, 0, -content_rect.height()/2);
//         p.drawText(r, Qt::TextSingleLine, p.fontMetrics().elidedText(synopsis_, Qt::ElideMiddle, r.width()));
//     }
//     else
//     {
//         if (!completion_.isNull())
//         {
//             if (completion_.startsWith(text(), Qt::CaseInsensitive))
//             {
//                 auto t = completion_.mid(text().length());
//                 auto r = content_rect.adjusted(horizontal_advance_, 0, 0, 0);
//                 p.drawText(r, Qt::TextSingleLine, o.fontMetrics.elidedText(t, Qt::ElideMiddle, r.width()));
//             }
//             else
//             {
//                 auto r = content_rect.adjusted(horizontal_advance_ + 5, 0, 0, 0);
//                 p.drawText(r, Qt::TextSingleLine, o.fontMetrics.elidedText(completion_, Qt::ElideMiddle, r.width()));
//             }
//         }
//         else // synopsis_
//         {
//             QFont font = p.font();
//             font.setItalic(true);
//             p.setFont(font);

//             auto r = content_rect.adjusted(horizontal_advance_ + 5, 0, 0, 0);
//             p.drawText(r, Qt::TextSingleLine, o.fontMetrics.elidedText(synopsis_, Qt::ElideRight, r.width()));
//         }
//     }
//     // TODO glitches when line is overflowing, need something better than QLineEdit, maybe QTextEdit
//     // if (trigger_length_)
//     // {
//     //     auto f = p.font();
//     //     f.setUnderline(true);
//     //     p.setFont(f);
//     //     p.drawText(content_rect, Qt::TextSingleLine, text().left(trigger_length_));
//     // }










// QSize InputLine::sizeHint() const
// {
//     QSize sizehint = QTextEdit::sizeHint();
//     // sizehint.setHeight(fitted_height);
//     INFO << "QTextEdit::sizeHint()" << sizehint;
//     INFO << "viewportSizeHint()" << viewportSizeHint();
//     INFO << "viewport()->size()"<< viewport()->size();
//     INFO << "InputLine::size()" << size();
//     return sizehint;
//     return {10,10};
// }

// QSize InputLine::viewportSizeHint() const
// {
//     return {10,10};
// }

// bool InputLine::event(QEvent *event)
// {
//     if (event->type() == QEvent::Polish)
//     if (event->type() == QEvent::Polish)
//         CRIT << "POLISH";
//     if (event->type() == QEvent::FontChange
//         ||event->type() == QEvent::Show)
//     {
//         CRIT << "FONT CCHANFGED";
//         // document()->adjustSize();

//         INFO << "InputLine::size()" << size();
//         INFO << "viewport()->contentsMargins()"<< viewport()->contentsMargins();
//         INFO << "viewport()->contentsRect() "<< viewport()->contentsRect();
//         INFO << "viewport()->frameSize() "<< viewport()->frameSize();
//         INFO << "viewport()->size()"<< viewport()->size();
//         INFO << "viewportMargins()"<< viewportMargins();
//         INFO << "viewportSizeHint()" << viewportSizeHint();
//         INFO << "INPUTLINE fm lineSpacing" << fontMetrics().lineSpacing();
//         INFO << "INPUTLINE doc height" << document()->size().height();
//         INFO << "INPUTLINE doc margin" << document()->documentMargin();
//         int height = fontMetrics().lineSpacing() + 2 * document()->documentMargin();
//         INFO << "Final height" << height;
//         INFO << "cursorRect(textCursor()).height()"<< cursorRect(textCursor()).height();
//         INFO << "font.pointSize()" << font().pointSize();
//         setFixedHeight(height);
//     }
//     return QTextEdit::event(event);
