//// Copyright (c) 2022 Manuel Schneider

#include "QtCore/qeventloop.h"
#include "QtWidgets/qapplication.h"
#include "src/fsindex.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_COLORS_ANSI
#include "albert/logging.h"
#include "doctest/doctest.h"
#include <QDebug>
#include <QFile>
#include <QTemporaryDir>
#include "src/fsindexpath.h"
using namespace std;
ALBERT_LOGGING_CATEGORY("files_test")

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


    //  --   --   --  //


    vector<shared_ptr<AbstractFileItem>> items;
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

    // namefilters
    p->setNameFilters({"b"});
    update();
    CHECK(items.size() == 4);


    // namefilters
    p->setNameFilters({"b"});
    update();
    CHECK(items.size() == 4);


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

    QApplication app(argc, &argv);

    QTemporaryDir root;
    CHECK(root.isValid());

    QDir dir(root.path());
    CHECK(dir.mkdir("a"));
    CHECK(dir.mkdir("b"));

    FsIndex fsi;
    QObject::connect(&fsi, &FsIndex::updatedFinished, &app, &QApplication::quit);
    auto dead_holder = make_unique<FsIndexPath>(root.path());
    auto *fsp = dead_holder.get();
    fsi.addPath(::move(dead_holder));

    CHECK(fsi.indexPaths().size() == 1);
    CHECK(fsi.indexPaths().contains(root.path()));
    CHECK(fsi.indexPaths().at(root.path()).get() == fsp);

    app.processEvents();





}
