//// Copyright (c) 2022-2024 Manuel Schneider

#define DOCTEST_CONFIG_COLORS_ANSI
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "fileitems.h"
#include "fsindex.h"
#include "fsindexpath.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTemporaryDir>
#undef WARN
#undef INFO
#include <albert/logging.h>
ALBERT_LOGGING_CATEGORY("files_test")
using namespace std;

TEST_CASE("FsIndexPath")
{
    QTemporaryDir root;
    CHECK(root.isValid());

    QDir dir(root.path());
    CHECK(dir.mkdir("a"));
    CHECK(dir.mkdir("b"));

    QFile file;

    auto createFile = [&](const QString &path){
        file.setFileName(path);
        CHECK(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("test");
        file.close();
    };

    createFile(root.filePath("a/foo.txt"));
    createFile(root.filePath("a/.foo.txt"));

    CHECK(QFile::link(root.filePath("a"),
                      root.filePath("b/c")));

    // ─┬─── /
    //  ├─┬─ a/
    //  │ ├─ a/.foo.txt
    //  │ └─ a/.foo.txt
    //  └─┬─ b/
    //    └─ b/c/

    vector<shared_ptr<FileItem>> items;
    FsIndexPath *p;

    auto update = [&](){
        items.clear();
        p->update(false, [](const QString &s){ qDebug() << s; });
        p->items(items);
    };

    p = new FsIndexPath(root.path());
    update();
    CHECK(items.size() == 1);

    // mimefilter dir
    p->setMimeFilters({"inode/directory"});
    update();
    CHECK(items.size() == 4);

    // mimefilter other
    p->setMimeFilters({"inode/directory", "text/plain"});
    update();
    CHECK(items.size() == 5);

    // depth
    p->setMaxDepth(0);
    update();
    CHECK(items.size() == 1);
    p->setMaxDepth(1);
    update();
    CHECK(items.size() == 3);
    p->setMaxDepth(2);
    update();
    CHECK(items.size() == 5);
    p->setMaxDepth(3);
    update();
    CHECK(items.size() == 5);

    // hidden
    p->setIndexHidden(true);
    update();
    CHECK(items.size() == 6);

    // symlink duplicate
    p->setFollowSymlinks(true);
    update();
    CHECK(items.size() == 6);
    p->setFollowSymlinks(false);

    // namefilters
    p->setNameFilters({"b"});
    update();
    CHECK(items.size() == 4);

    p->setNameFilters({"^a"});
    update();
    CHECK(items.size() == 3);

    p = new FsIndexPath(dir.filePath("b"));
    update();
    CHECK(items.size() == 1);

    p->setFollowSymlinks(true);
    update();
    CHECK(items.size() == 1);

    // symlink
    p->setMimeFilters({"text/plain"});
    update();
    CHECK(items.size() == 2);

    // symlink + hidden
    p->setIndexHidden(true);
    update();
    CHECK(items.size() == 3);

    // with dirs
    p->setMimeFilters({"inode/directory", "text/plain"});
    update();
    CHECK(items.size() == 4);

    // with dirs no hidden
    p->setIndexHidden(false);
    update();
    CHECK(items.size() == 3);
}

TEST_CASE("FsIndex")
{
    int argc;
    char *argv = {};

    QCoreApplication app(argc, &argv);
    QLoggingCategory::setFilterRules("*.debug=true");

    QTemporaryDir root;
    CHECK(root.isValid());

    QDir dir(root.path());
    CHECK(dir.mkdir("a"));
    CHECK(dir.mkdir("b"));

    FsIndex fsi;
    QObject::connect(&fsi, &FsIndex::updatedFinished, &app, [](){
        QCoreApplication::processEvents();
        QCoreApplication::quit();
    });
    QObject::connect(&fsi, &FsIndex::status, &app, [](const QString &s){ qDebug() << s; });

    auto dead_holder = make_unique<FsIndexPath>(root.path());
    auto *fsp = dead_holder.get();
    fsp->setMimeFilters({"inode/directory"});
    fsi.addPath(::move(dead_holder));

    CHECK(fsi.indexPaths().size() == 1);
    CHECK(fsi.indexPaths().contains(root.path()));
    CHECK(fsi.indexPaths().at(root.path()).get() == fsp);

    app.processEvents();

    vector<shared_ptr<FileItem>> items;
    QThread::sleep(2); // Sleep some time such that the mdates differ at all
    fsp->items(items);

    CHECK(items.size() == 3);
    CHECK(dir.mkdir("b/c"));

    QTimer::singleShot(0, &app, [&](){fsi.update();});
    app.exec();

    items.clear();
    QThread::sleep(2); // Sleep some time such that the mdates differ at all
    fsp->items(items);
    CHECK(items.size() == 4);

}
