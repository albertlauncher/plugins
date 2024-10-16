//// Copyright (c) 2022-2024 Manuel Schneider

#include "fileitems.h"
#include "fsindex.h"
#include "fsindexpath.h"
#include "test.h"
#include <QFile>
#include <QTemporaryDir>
using namespace std;


QTEST_MAIN(Test)


void Test::fs_index_path()
{
    QTemporaryDir root;
    QVERIFY(root.isValid());

    QDir dir(root.path());
    QVERIFY(dir.mkdir("a"));
    QVERIFY(dir.mkdir("b"));

    QFile file;

    auto createFile = [&](const QString &path)
    {
        file.setFileName(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("test");
        file.close();
    };

    createFile(root.filePath("a/foo.txt"));
    createFile(root.filePath("a/.foo.txt"));

    QVERIFY(QFile::link(root.filePath("a"), root.filePath("b/c")));

    // ─┬─── /
    //  ├─┬─ a/
    //  │ ├─ a/.foo.txt
    //  │ └─ a/.foo.txt
    //  └─┬─ b/
    //    └─ b/c/

    vector<shared_ptr<FileItem>> items;
    FsIndexPath *p;

    auto update = [&]()
    {
        items.clear();
        p->update(false, [](const QString &s) { qDebug() << s; });
        p->items(items);
    };

    p = new FsIndexPath(root.path());
    update();
    QCOMPARE(items.size(), 1);

    // mimefilter dir
    p->setMimeFilters({"inode/directory"});
    update();
    QCOMPARE(items.size(), 4);

    // mimefilter other
    p->setMimeFilters({"inode/directory", "text/plain"});
    update();
    QCOMPARE(items.size(),  5);

    // depth
    p->setMaxDepth(0);
    update();
    QCOMPARE(items.size(),  1);
    p->setMaxDepth(1);
    update();
    QCOMPARE(items.size(),  3);
    p->setMaxDepth(2);
    update();
    QCOMPARE(items.size(),  5);
    p->setMaxDepth(3);
    update();
    QCOMPARE(items.size(),  5);

    // hidden
    p->setIndexHidden(true);
    update();
    QCOMPARE(items.size(),  6);

    // symlink duplicate
    p->setFollowSymlinks(true);
    update();
    QCOMPARE(items.size(),  6);
    p->setFollowSymlinks(false);

    // namefilters
    p->setNameFilters({"b"});
    update();
    QCOMPARE(items.size(),  4);

    p->setNameFilters({"^a"});
    update();
    QCOMPARE(items.size(),  3);

    p = new FsIndexPath(dir.filePath("b"));
    update();
    QCOMPARE(items.size(),  1);

    p->setFollowSymlinks(true);
    update();
    QCOMPARE(items.size(),  1);

    // symlink
    p->setMimeFilters({"text/plain"});
    update();
    QCOMPARE(items.size(),  2);

    // symlink + hidden
    p->setIndexHidden(true);
    update();
    QCOMPARE(items.size(),  3);

    // with dirs
    p->setMimeFilters({"inode/directory", "text/plain"});
    update();
    QCOMPARE(items.size(),  4);

    // with dirs no hidden
    p->setIndexHidden(false);
    update();
    QCOMPARE(items.size(),  3);
}

void Test::fs_index()
{
    int argc;
    char *argv = {};

    // QCoreApplication app(argc, &argv);
    QLoggingCategory::setFilterRules("*.debug=true");

    QTemporaryDir root;
    QVERIFY(root.isValid());

    QDir dir(root.path());
    QVERIFY(dir.mkdir("a"));
    QVERIFY(dir.mkdir("b"));

    FsIndex fsi;
    QObject::connect(&fsi, &FsIndex::status,
                     qApp, [](const auto &s) { qDebug() << s; });

    auto dead_holder = make_unique<FsIndexPath>(root.path());
    auto *fsp = dead_holder.get();
    fsp->setMimeFilters({"inode/directory"});
    fsi.addPath(::move(dead_holder));

    QCOMPARE(fsi.indexPaths().size(),  1);
    QVERIFY(fsi.indexPaths().contains(root.path()));
    QCOMPARE(fsi.indexPaths().at(root.path()).get(),  fsp);

    // qApp->processEvents();

    vector<shared_ptr<FileItem>> items;
    QThread::sleep(2); // Sleep some time such that the mdates differ at all
    fsp->items(items);

    QCOMPARE(items.size(),  3);
    QVERIFY(dir.mkdir("b/c"));

    QObject::connect(&fsi, &FsIndex::updatedFinished,
                     qApp, []{ QCoreApplication::quit(); });  // Quit the loop on finished

    QTimer::singleShot(0, qApp, [&]() { fsi.update(); });
    qApp->exec();

    items.clear();
    QThread::sleep(2); // Sleep some time such that the mdates differ at all
    fsp->items(items);
    QCOMPARE(items.size(),  4);
}
