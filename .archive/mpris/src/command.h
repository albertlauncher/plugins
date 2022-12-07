// Copyright (c) 2022 Manuel Schneider

#pragma once

#include <QString>
#include <QVariant>
#include <functional>
#include "albert.h"
#include "player.h"
using std::function;
typedef std::shared_ptr<albert::Item> SharedItem;

class Command
{
public:
    /**
     * @brief Command   Constructs a DBus command to launch from albert with the given parameters.
     * @param label     An internal variable to query at a later point. Not needed for the DBus query.
     * @param title     The title of the StandardItem which will be created.
     * @param subtext   The subtext of the StandardItem which will be created.
     * @param method    The DBus method to invoke when this command is performed.
     * @param iconpath  The path to the icon which the StandardItem will get.
     */
    Command(const QString& label, const QString& title, const QString& subtext, const QString& method, QStringList icon);

    QString& getLabel();
    QString& getTitle();
    QString& getMethod();
    QStringList &getIcon();

    /**
     * @brief applicableWhen    Configure this command to be only appicable under a certian (given) conditions.
     * @param path              The path to query the property from.
     * @param property          The name of the property. This property will be checked as condition.
     * @param expectedValue     The value of the property.
     * @param positivity        The result of the equality-check (queriedValue == expectedValue). Here you can negate the result.
     * @return                  Returns itself, but now configured for applicability-check
     */
    Command& applicableWhen(const char *path, const char* property, const QVariant expectedValue, bool positivity);

    /**
     * @brief produceStandardItem   Produces an instance of AlbertItem for this command to invoke on a given Player.
     * @return                      Returns a shared_ptr on this AlbertItem.
     */
    albert::SStdItem produceAlbertItem(Player &) const;

    /**
     * @brief isApplicable  If configured, checks if the given property meets the expected criteria.
     * @return              True if not configured or match, false if the property is different than expected.
     */
    bool isApplicable(Player&) const;

private:
    QString label_, title_, subtext_, method_;
    QStringList icon_;
    bool applicableCheck_;
    QString path_;
    QString property_;
    QVariant expectedValue_;
    bool positivity_;
};
