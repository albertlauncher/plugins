// Copyright (c) 2022-2025 Manuel Schneider

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
                      input_line.trigger_color_);

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

    connect(this, &QPlainTextEdit::textChanged,
            this, &InputLine::textEdited);

    connect(this, &InputLine::textEdited, this, [this]{
        history_.resetIterator();
        user_text_ = text();
    });

    // auto fixHeight = [this]{
    //     INFO << "INPUTLINE fm lineSpacing" << fontMetrics().lineSpacing();
    //     INFO << "INPUTLINE doc height" << document()->size().height();
    //     INFO << "INPUTLINE doc margin" << document()->documentMargin();
    //     int height = document()->size().height() ;//+ document()->documentMargin();
    //     INFO << "Final height" << height;
    //     setFixedHeight(height);
    // };

    connect(document()->documentLayout(),
            &QAbstractTextDocumentLayout::documentSizeChanged,
            this,
            [this](const QSizeF &newSize){
                setFixedHeight((int)newSize.height() * fontMetrics().lineSpacing()
                               + 2 * (int)document()->documentMargin()); });
}

const QString &InputLine::synopsis() const { return synopsis_; }

void InputLine::setSynopsis(const QString &t)
{
    synopsis_ = t;
    update();
}

const QString &InputLine::completion() const { return completion_; }

void InputLine::setCompletion(const QString &t)
{
    completion_ = t;
    update();
}

uint InputLine::triggerLength() const { return trigger_length_; }

void InputLine::setTriggerLength(uint len)
{
    trigger_length_ = len;
    QSignalBlocker b(this);  // see below
    highlighter_->rehighlight();  // triggers QPlainTextEdit::textChanged!
}

QString InputLine::text() const { return toPlainText(); }

void InputLine::setText(QString t)
{
    // setPlainText(t);  // Dont. Clears undo stack.

    disconnect(this, &QPlainTextEdit::textChanged, this, &InputLine::textEdited);

    QTextCursor c{document()};
    c.beginEditBlock();
    c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    c.removeSelectedText();
    c.insertText(t);
    c.endEditBlock();
    setTextCursor(c);

    connect(this, &QPlainTextEdit::textChanged, this, &InputLine::textEdited);
}

uint InputLine::fontSize() const { return font().pointSize(); }

void InputLine::setFontSize(uint val)
{
    auto f = font();
    f.setPointSize(val);
    setFont(f);

    // setFixedHeight(fontMetrics().lineSpacing() + 2 * (int)document()->documentMargin());
}

QColor InputLine::triggerColor() const { return trigger_color_; }

void InputLine::setTriggerColor(const QColor &val)
{
    if (trigger_color_ == val)
        return;
    trigger_color_ = val;
    QSignalBlocker b(this);  // see below
    highlighter_->rehighlight();  // triggers QPlainTextEdit::textChanged!
}

QColor InputLine::hintColor() const { return hint_color_; }

void InputLine::setHintColor(const QColor &val)
{
    if (hint_color_ == val)
        return;
    hint_color_ = val;
    update();
}

void InputLine::next()
{
    auto t = history_.next(history_search ? user_text_ : QString());

    // Without ClearOnHide the text is already in the input
    // I.e. the first item in history equals the input text
    if (t == text())
        t = history_.next(history_search ? user_text_ : QString());

    if (!t.isNull())
        setText(t);
}

void InputLine::previous()
{
    if (auto t = history_.prev(history_search ? user_text_ : QString());
        !t.isNull())
        setText(t);
}

void InputLine::paintEvent(QPaintEvent *event)
{
    if (document()->size().height() == 1
        && !(synopsis_.isEmpty() && completion_.isEmpty()))
    {

        QString c = completion();
        if (auto query = text().mid(trigger_length_);
            completion().startsWith(query, Qt::CaseInsensitive))
            c = completion().mid(query.length());
        else
            c.prepend(QChar::Space);

        auto r = QRectF(contentsRect()).adjusted(fontMetrics().horizontalAdvance(text()) + 1,
                                                 1, -1, -1); // 1xp document margin
        auto c_width = fontMetrics().horizontalAdvance(c);
        if (c_width > r.width())
        {
            c = fontMetrics().elidedText(c, Qt::ElideRight, (int)r.width());
            c_width = fontMetrics().horizontalAdvance(c);
        }

        QPainter p(viewport());
        p.setPen(hint_color_);
        p.drawText(r, Qt::TextSingleLine, c);

        if (fontMetrics().horizontalAdvance(synopsis()) + c_width < r.width())
            p.drawText(r.adjusted(c_width, 0, 0, 0),
                       Qt::TextSingleLine | Qt::AlignRight,
                       synopsis());
    }


    // qreal bearing_diff = 0;
    // if (!text().isEmpty())
    // {
    //     QChar c = text().at(text().length()-1);
    //     DEBG << "rightBearing" << c << QFontMetricsF(font()).rightBearing(c);
    //     // bearing_diff += QFontMetricsF(font()).rightBearing(c);
    // }
    // if (!hint.isEmpty())
    // {
    //     DEBG << "leftBearing" << hint.at(0) << QFontMetricsF(font()).leftBearing(hint.at(0));
    //     // bearing_diff += QFontMetricsF(font()).leftBearing(hint.at(0));
    // }
    // r.adjust(bearing_diff, 0, 0, 0);

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

    case Qt::Key_Tab:
        if (!completion_.isEmpty())
        {
            setText(text().left(trigger_length_) + completion_);
            return event->accept();
        }

    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (event->modifiers() == Qt::NoModifier)
            return event->ignore();
        else if (event->modifiers().testFlag(Qt::ShiftModifier))
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

void InputLine::inputMethodEvent(QInputMethodEvent *event)
{
    if (disable_input_method_ && !event->preeditString().isEmpty()) {
        qApp->inputMethod()->commit();
        return event->accept();
    }
    else
        QPlainTextEdit::inputMethodEvent(event);

}
