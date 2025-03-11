// Copyright (c) 2022-2024 Manuel Schneider

#include "inputline.h"
#include <QApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QSyntaxHighlighter>
#include <albert/logging.h>

class InputLine::TriggerHighlighter : public QSyntaxHighlighter
{
    InputLine &input_line;
public:
    TriggerHighlighter(QTextDocument *d, InputLine *i) : QSyntaxHighlighter(d), input_line(*i) {}

    void highlightBlock(const QString &text) override
    {
        if (input_line.trigger_length_ && text.length() >= input_line.trigger_length_)
        {
            setFormat(0, input_line.trigger_length_,
                      input_line.palette().highlight().color());

            setFormat(input_line.trigger_length_,
                      text.length()-input_line.trigger_length_,
                      input_line.palette().text().color());
        }
    }
};


InputLine::InputLine(QWidget *parent):
    QPlainTextEdit(parent),
    highlighter_(new TriggerHighlighter(document(), this))
{
    document()->setDocumentMargin(1); // 0 would be optimal but clips bearing

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
            this, [this](const QSizeF &newSize){
        CRIT << newSize;
        // setFixedHeight((int)newSize.height());
        setFixedHeight((int)newSize.height()*fontMetrics().lineSpacing() + 2 * (int)document()->documentMargin());

    });

    connect(this, &QPlainTextEdit::textChanged, this, [this]
    {
        // Save and emit plainTextChanged if necessary
        auto t = toPlainText();
        if (t != text_)
        {
            text_ = t;
            emit textChanged(text_);
        }

        input_hint_.clear();
    });
}

void InputLine::setInputHint(const QString &t)
{
    input_hint_ = t;
    update();
}

void InputLine::setCompletion(const QString &t)
{
    completion_ = t.toLower();
    update();
}

void InputLine::setTriggerLength(uint len)
{
    trigger_length_ = len;
    highlighter_->rehighlight();
}

QString InputLine::text() const { return toPlainText(); }

void InputLine::setText(QString t)
{
    // setPlainText(t);  // Dont. Clears undo stack.

    QTextCursor c{document()};
    c.beginEditBlock();
    c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    c.removeSelectedText();
    c.insertText(t);
    c.endEditBlock();
    setTextCursor(c);
}

uint InputLine::fontSize() const { return font().pointSize(); }

void InputLine::setFontSize(uint val)
{
    auto f = font();
    f.setPointSize(val);
    setFont(f);

    // setFixedHeight(fontMetrics().lineSpacing() + 2 * (int)document()->documentMargin());
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

void InputLine::paintEvent(QPaintEvent *event)
{
    if (!input_hint_.isNull() || !completion_.isNull())
    {
        auto r = QRectF(contentsRect()).adjusted(
            QFontMetricsF(font()).horizontalAdvance(text()) + 1, 1, -1, -1); // 1xp document margin

        QString hint = input_hint_.isNull() ? completion_ : input_hint_;
        if (auto query = text().mid(trigger_length_); hint.startsWith(query, Qt::CaseInsensitive))
        {
            hint = hint.mid(query.length());

            qreal bearing_diff = 0;
            if (!text().isEmpty())
            {
                QChar c = text().at(text().length()-1);
                DEBG << "rightBearing" << c << QFontMetricsF(font()).rightBearing(c);
                // bearing_diff -= QFontMetricsF(font()).rightBearing(c);
            }
            if (!hint.isEmpty())
            {
                DEBG << "leftBearing" << hint.at(0) << QFontMetricsF(font()).leftBearing(hint.at(0));
                // bearing_diff -= QFontMetricsF(font()).leftBearing(hint.at(0));
            }
            r.adjust(bearing_diff, 0, 0, 0);
        }
        else
            hint = QString(" %1").arg(input_hint_);


        // drawDebugRect(p, r,
        //               QString("%1 inputhint").arg(objectName()),
        //               Qt::red, Qt::green);

        QPainter p(viewport());
        p.setPen(palette().button().color());
        p.drawText(r, Qt::TextSingleLine,
                   fontMetrics().elidedText(hint, Qt::ElideRight, (int)r.width()));
    }

    QPlainTextEdit::paintEvent(event);
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

    QPlainTextEdit::hideEvent(event);
}

void InputLine::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {

    // case Qt::Key_Tab:
    //     if (!completion_.isEmpty())
    //         setText(completion_);
    //     return event->accept();

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (event->modifiers().testFlag(Qt::ShiftModifier))
        {
            insertPlainText("\n");
            return event->accept();
        }

#if defined Q_OS_MACOS
    case Qt::Key_Backspace:
        if (event->modifiers().testFlag(Qt::ControlModifier))
        {
            QTextCursor c = textCursor();
            c.beginEditBlock();
            c.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
            c.removeSelectedText();
            c.endEditBlock();
            return event->accept();
        }
#endif

    default:
        break;
    }

    QPlainTextEdit::keyPressEvent(event);
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
