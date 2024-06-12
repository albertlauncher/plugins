// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <albert/inputhistory.h>
#include <QPlainTextEdit>
class Style;

// Input method problems: https://bugreports.qt.io/browse/QTBUG-106516
class InputLine : public QTextEdit
{
    Q_OBJECT

public:

    InputLine(QWidget *parent = nullptr);

    // void setSynopsis(const QString&);
    void setCompletion(const QString&);
    void setTriggerLength(uint);

    bool clear_on_hide;
    bool history_search;

    QString text() const;
    void setText(QString);

    void next();
    void previous();

    void setStyle(const Style*);

private:

    // bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void colorizeTrigger();
    void updateTooltip();
    // QSize sizeHint() const override;
    // QSize viewportSizeHint() const override;
    // uint fitted_height;

    const Style *style_;
    albert::InputHistory history_;
    // QString synopsis_;
    QString completion_;
    QString user_text_; // used for history search
    uint trigger_length_;
    QString plain_text_;

signals:

    void plainTextChanged(const QString&);

};


