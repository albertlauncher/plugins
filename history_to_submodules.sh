#! /usr/bin/env bash

#set -x
set -e

# Find history dirs
# git log --pretty=format: --name-only  | grep -E '^(.archive/)?[^/]+' -o | sort -u | uniq

git clone --bare git@github.com:albertlauncher/plugins.git .bare || true

function filter (

    if [[ $# -lt 2 ]]; then
        echo "Error: At least two arguments required."
        return 1
    fi

    local repo_name="$1"
    local filter_path="$2"
    shift 2

    declare -a additional_filter_paths
    if [[ $# -gt 0 ]]; then
        additional_filter_paths=$(printf -- "--path %s " "$@")
    fi

    echo "ℹ️" $repo_name $filter_path $additional_filter_paths

    git clone .bare $repo_name
    cd $repo_name
    git filter-repo --subdirectory-filter ${filter_path} ${additional_filter_paths[@]}
    git remote add origin git@github.com:albertlauncher/albert-plugin-$repo_name.git
    git push --set-upstream origin main -f
    cd ..
)


#        reponame                 path                    additional paths
####################################################################################################
filter   applications             applications                  applauncher applications_macos applications_xdg
filter   bluetooth                bluetooth
filter   chromium                 chromium                      bookmarkindex chromebookmarks
filter   caffeine                 caffeine
filter   calculator-qalculate     calculator_qalculate          qalculator
filter   clipboard                clipboard
filter   contacts                 contacts                      contacts_macos
filter   datetime                 datetime
filter   debug                    debug
filter   dictionary               dictionary                    platform_macos
filter   docs                     docs
filter   files                    files                         fileindex
filter   hash                     hash                          hashgenerator
filter   menubar                  menubar
filter   mpris                    mpris
filter   path                     path                          terminal
filter   python                   python
filter   snippets                 snippets                      kvstore
filter   ssh                      ssh
filter   system                   system
filter   timer                    timer
filter   timezones                timezones
filter   urlhandler               urlhandler
filter   vpn                      vpn
filter   websearch                websearch
filter   widgetsboxmodel          widgetsboxmodel               widgetboxmodel

filter   calculator-exprtk        .archive/calculator_exprtk
filter   calculator-muparser      .archive/calculator_muparser  calculator
filter   external-extensions      .archive/externalextensions   externalextensions
filter   firefox                  .archive/firefoxbookmarks     firefoxbookmarks
filter   qmlboxmodel              .archive/qmlboxmodel          qmlboxmodel
filter   sparkle-updater          .archive/sparkle_updater      sparkle_updater
filter   telegram                 .archive/telegram
filter   virtualbox               .archive/virtualbox           virtualbox
filter   wifi                     .archive/wifi
