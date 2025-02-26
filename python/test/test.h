// Copyright (c) 2022-2024 Manuel Schneider
#include <QCoreApplication>

class PythonTests : public QObject
{
    Q_OBJECT

private slots:

    void initTestCase();

    void testAction();
    void testItem();
    void testStandardItem();
    void testMatcher();

    void testTriggerQueryHandler();
    void testGlobalQueryHandler();
    void testIndexQueryHandler();
    void testFallbackQueryHandler();

};
