// Copyright (c) 2022-2024 Manuel Schneider
#include <QCoreApplication>
#include <QtTest/QtTest>

class Test : public QObject
{
    Q_OBJECT

private slots:

    void fs_index_path();
    void fs_index();

};
