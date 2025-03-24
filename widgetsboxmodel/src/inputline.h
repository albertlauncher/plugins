// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include <QPlainTextEdit>
#include <albert/inputhistory.h>

class InputLine : public QPlainTextEdit
{
    Q_OBJECT

public:

    InputLine(QWidget *parent = nullptr);

    const QString& synopsis() const;
    void setSynopsis(const QString&);

    const QString& completion() const;
    void setCompletion(const QString& = {});

    uint triggerLength() const;
    void setTriggerLength(uint);

    QString text() const;
    void setText(QString);

    uint fontSize() const;
    void setFontSize(uint);

    QColor hintColor() const;
    void setHintColor(const QColor &);

    void next();
    void previous();

    bool clear_on_hide;
    bool history_search;
    bool disable_input_method_;

private:

    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void inputMethodEvent(QInputMethodEvent *event) override;

    albert::InputHistory history_;
    QString completion_;
    QString synopsis_;
    QString user_text_;
    uint trigger_length_;
    class TriggerHighlighter;
    TriggerHighlighter *highlighter_;
    QColor hint_color_;

signals:

    void textEdited();

};


