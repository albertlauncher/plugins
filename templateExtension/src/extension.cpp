// Copyright (C) 2014-2020 Manuel Schneider

#include <QPointer>
#include <stdexcept>
#include "albert/util/standarditem.h"
#include "configwidget.h"
#include "extension.h"
#include "xdg/iconlookup.h"
Q_LOGGING_CATEGORY(qlc, "template")
#define DEBG qCDebug(qlc,).noquote()
#define INFO qCInfo(qlc,).noquote()
#define WARN qCWarning(qlc,).noquote()
#define CRIT qCCritical(qlc,).noquote()
using namespace Core;
using namespace std;

class ProjectNamespace::Private
{
public:
    QPointer<ConfigWidget> widget;
};


/** ***************************************************************************/
ProjectNamespace::Extension::Extension()
    : Core::Extension("org.albert.extension.projectid"), // Must match the id in metadata
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    registerQueryHandler(this);

    // You can throw in the constructor if something fatal happened
    // throw std::runtime_error( "Description of error." );
    // throw std::string( "Description of error." );
    // throw QString( "Description of error." );
    // throw "Description of error.";
    // throw; // Whatever prints "unknown error"
}



/** ***************************************************************************/
ProjectNamespace::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *ProjectNamespace::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);
    }
    return d->widget;
}



/** ***************************************************************************/
void ProjectNamespace::Extension::setupSession() {

}



/** ***************************************************************************/
void ProjectNamespace::Extension::teardownSession() {

}



/** ***************************************************************************/
void ProjectNamespace::Extension::handleQuery(Core::Query *) const {

    /*
     * Things change so often I wont maintain this tutorial here. Check the relevant headers.
     *
     * - core/extension.h
     * - core/queryhandler.h
     * - core/query.h
     * - core/item.h
     * - core/action.h
     * - util/standarditem.h
     * - util/offlineindex.h
     * - util/standardindexitem.h
     *
     * Use
     *
     *   query->addMatch(my_item)
     *
     * to add matches. If you created a throw away item MOVE it instead of
     * copying e.g.:
     *
     *   query->addMatch(std::move(my_tmp_item))
     *
     * The relevance factor is optional. (Defaults to 0) its a usigned integer depicting the
     * relevance of the item 0 mean not relevant UINT_MAX is totally relevant (exact match).
     * E.g. it the query is "it" and your items name is "item"
     *
     *   my_item.name().startswith(query->string)
     *
     * is a naive match criterion and
     *
     *   UINT_MAX / ( query.searchterm().size() / my_item.name().size() )
     *
     * a naive match factor.
     *
     * If you have a lot of items use the iterator versions addMatches, e.g. like that
     *
     *   query->addMatches(my_items.begin(), my_items.end());
     *
     * If the items in the container are temporary object move them to avoid uneccesary
     * reference counting:
     *
     *   query->addMatches(std::make_move_iterator(my_tmp_items.begin()),
     *                     std::make_move_iterator(my_tmp_items.end()));
     */
}

