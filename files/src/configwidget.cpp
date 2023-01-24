// Copyright (c) 2022-2023 Manuel Schneider

#include "configwidget.h"
#include "mimefilterdialog.h"
#include "namefilterdialog.h"
#include "plugin.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <map>
using namespace std;

static QStringList getPaths(const map<QString,unique_ptr<FsIndexPath>> &index_paths){
    QStringList paths;
    for (const auto &[p,_] : index_paths)
        paths << p;
    return paths;
};

ConfigWidget::ConfigWidget(Plugin *plu, QWidget *par) : QWidget(par), plugin(plu)
{
    ui.setupUi(this);

    auto &index_paths = plu->fsIndex().indexPaths();
    paths_model.setStringList(getPaths(index_paths));
    ui.listView_paths->setModel(&paths_model);

    connect(ui.toolButton_add, &QPushButton::clicked, this, [this]()
    {
        QString path = QFileDialog::getExistingDirectory(
                this,
                tr("Choose directory"),
                QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        if (!path.isEmpty()){
            auto *p = new FsIndexPath(path);
            if (plugin->fsIndex().addPath(p)){
                plugin->fsIndex().update(p);
                paths_model.setStringList(getPaths(plugin->fsIndex().indexPaths()));
                for (int i = 0; i < paths_model.stringList().size(); ++i)
                    if (paths_model.stringList()[i] == path)
                        ui.listView_paths->setCurrentIndex(paths_model.index(i, 0));
            }
            else delete p;
        }
        ui.listView_paths->setFixedHeight(
                ui.listView_paths->contentsMargins().bottom() +
                ui.listView_paths->contentsMargins().top() +
                paths_model.rowCount()*ui.listView_paths->sizeHintForRow(0));
    });

    connect(ui.toolButton_rem, &QPushButton::clicked, this, [this]()
    {
        if (ui.listView_paths->currentIndex().isValid()){
            plugin->fsIndex().remPath(ui.listView_paths->currentIndex().data().toString());
            paths_model.removeRow(ui.listView_paths->currentIndex().row());
        }
        ui.listView_paths->setFixedHeight(
            ui.listView_paths->contentsMargins().bottom() +
            ui.listView_paths->contentsMargins().top() +
            paths_model.rowCount()*ui.listView_paths->sizeHintForRow(0));
    });

    connect(plugin, &Plugin::statusInfo, this, [this](const QString& text){
        QFontMetrics metrics(ui.label_statusbar->font());
        QString elidedText = metrics.elidedText(text, Qt::ElideRight, ui.label_statusbar->width()-5);
        ui.label_statusbar->setText(elidedText);
    });

    /*
     * Per path stuff
     */

    // Update ui on index change
    connect(ui.listView_paths->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex &current, const QModelIndex&){
                if (!current.isValid()) {
                    ui.groupBox_path->setEnabled(false);
                } else {
                    ui.groupBox_path->setEnabled(true);
                    current_path = current.data().toString();
                    auto &fsp = plugin->fsIndex().indexPaths().at(current_path);
                    ui.checkBox_hidden->setChecked(fsp->indexHidden());
                    ui.checkBox_followSymlinks->setChecked(fsp->followSymlinks());
                    ui.spinBox_depth->setValue(static_cast<int>(fsp->maxDepth()));
                    ui.spinBox_interval->setValue(static_cast<int>(fsp->scanInterval()));
                    ui.checkBox_fswatch->setChecked(fsp->watchFileSystem());
                    adjustMimeCheckboxes();
                }
            });

    connect(ui.pushButton_namefilters, &QPushButton::clicked, this, [this]() {
        auto &fsp = plugin->fsIndex().indexPaths().at(current_path);
        NameFilterDialog dialog(fsp->nameFilters(), this);
        dialog.setWindowModality(Qt::WindowModal);
        if (dialog.exec()) {
            auto filters = dialog.filters();
            filters.removeDuplicates();
            fsp->setNameFilters(filters);
        }
    });

    connect(ui.pushButton_mimefilters, &QPushButton::clicked, this, [this]() {
        auto &fsp = plugin->fsIndex().indexPaths().at(current_path);
        MimeFilterDialog dialog(fsp->mimeFilters(), this);
        dialog.setWindowModality(Qt::WindowModal);
        if (dialog.exec()) {
            auto filters = dialog.filters();
            filters.removeDuplicates();
            fsp->setMimeFilters(filters);
            adjustMimeCheckboxes();

        }
    });

    connect(ui.checkBox_hidden, &QCheckBox::clicked, this,
            [this](bool value){ plugin->fsIndex().indexPaths().at(current_path)->setIndexHidden(value); });

    connect(ui.checkBox_followSymlinks, &QCheckBox::clicked, this,
            [this](bool value){ plugin->fsIndex().indexPaths().at(current_path)->setFollowSymlinks(value); });

    connect(ui.spinBox_interval, &QSpinBox::valueChanged, this,
            [this](int value){ plugin->fsIndex().indexPaths().at(current_path)->setScanInterval(value); });

    connect(ui.spinBox_depth, &QSpinBox::valueChanged, this,
            [this](int value){ plugin->fsIndex().indexPaths().at(current_path)->setMaxDepth(value); });

    connect(ui.checkBox_fswatch, &QCheckBox::clicked, this,
            [this](bool value){
                if (value)
                    QMessageBox::warning(this, "Warning",
                                         "Enabling file system watches comes with caveats. "
                                         "You should only activate this option if you know "
                                         "what you are doing. A lot of file system changes "
                                         "(compilation, installing, etc) while having "
                                         "watches enabled can put your system under high "
                                         "load. You have been warned.");

                plugin->fsIndex().indexPaths().at(current_path)->setWatchFilesystem(value);
            });

    auto helper = [this](QCheckBox *checkbox, const QString& type){
        connect(checkbox, &QCheckBox::clicked, this, [this, checkbox, type](bool checked){
            checkbox->setTristate(false);
            auto patterns = plugin->fsIndex().indexPaths().at(current_path)->mimeFilters();
            patterns = patterns.filter(QRegularExpression(QString(R"(^(?!%1\/))").arg(type))); // drop all of mimetype class
            if (checked)
                patterns.push_back(QString("%1/*").arg(type));
            plugin->fsIndex().indexPaths().at(current_path)->setMimeFilters(patterns);
        });
    };
    helper(ui.checkBox_audio, "audio");
    helper(ui.checkBox_video, "video");
    helper(ui.checkBox_image, "image");
    helper(ui.checkBox_docs, "application");

    connect(ui.checkBox_dirs, &QCheckBox::clicked, this, [this](bool checked){
        auto patterns = plugin->fsIndex().indexPaths().at(current_path)->mimeFilters();
        patterns.removeAll("inode/directory");
        if (checked)
            patterns.push_back("inode/directory");
        plugin->fsIndex().indexPaths().at(current_path)->setMimeFilters(patterns);
    });

    ui.listView_paths->setFixedHeight(
            ui.listView_paths->contentsMargins().bottom() +
            ui.listView_paths->contentsMargins().top() +
            paths_model.rowCount()*ui.listView_paths->sizeHintForRow(0));
}

void ConfigWidget::adjustMimeCheckboxes()
{
    auto patterns = plugin->fsIndex().indexPaths().at(current_path)->mimeFilters();
    ui.checkBox_dirs->setCheckState(patterns.contains("inode/directory") ? Qt::Checked : Qt::Unchecked);
    map<QCheckBox*,QString> m {
            {ui.checkBox_audio, "audio/"},
            {ui.checkBox_video, "video/"},
            {ui.checkBox_image, "image/"},
            {ui.checkBox_docs, "application/"}
    };
    for (const auto &[checkbox, mime_prefix] : m){
        if (patterns.contains(mime_prefix+"*"))
            checkbox->setCheckState(Qt::Checked);
        else if (any_of(patterns.begin(), patterns.end(),
                        [mime_prefix=mime_prefix](const QString & str){ return str.startsWith(mime_prefix); }))
            checkbox->setCheckState(Qt::PartiallyChecked);
        else
            checkbox->setCheckState(Qt::Unchecked);
    }
}

