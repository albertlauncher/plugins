// Copyright (c) 2022-2024 Manuel Schneider

#include "colordialog.h"
#include "fontlisteditor.h"
#include "propertyeditor.h"
#include <QAbstractTableModel>
#include <QHeaderView>
#include <QItemEditorFactory>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QVBoxLayout>
#include <albert/logging.h>


class PropertyModel final : public QAbstractTableModel
{
public:
    PropertyModel(QObject *style_object, QObject *parent = nullptr):
        QAbstractTableModel(parent),
        style_object_(style_object)
    {
        auto *mo = style_object_->metaObject();
        for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
            auto p = mo->property(i);
            CRIT << p.typeName() << p.name()<<  p.read(style_object);
            properties_ << p.name();
        }
    }

    int rowCount(const QModelIndex&) const override
    { return properties_.count(); }

    int columnCount(const QModelIndex&) const override
    { return 2; }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            if ( index.column() == 0 )
            {
                auto string = properties_[index.row()];
                string.replace(0, 1, string[0].toUpper());
                string.replace("_", " ");
                return string;
            }
            else if (index.column()==1)
                return style_object_->property(properties_.at(index.row()).toLatin1().data());
        }
        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            if (section==0)
                return "Property";
            if (section==1)
                return "Value";
        }
        return {};
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
    {
        if (role ==  Qt::EditRole)
            return style_object_->setProperty(properties_[index.row()].toLatin1().data(), value);
        return false;
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (index.column() == 1)
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
        else
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

private:
    QObject *const style_object_;
    QStringList properties_;
};


PropertyEditor::PropertyEditor(QObject *style_object, QWidget *parent):
    QDialog(parent)
{
    resize(480, 480);
    setWindowTitle("PropertyEditor");

    QTableView *tableView = new QTableView(this);

    tableView->setAlternatingRowColors(true);
    tableView->setEditTriggers(QAbstractItemView::SelectedClicked|QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
    tableView->setModel(new PropertyModel(style_object, tableView));
    tableView->setObjectName(QStringLiteral("tableView"));
//    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setShowGrid(false);
    tableView->setWordWrap(false);

    tableView->horizontalHeader()->setMinimumSectionSize(16);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setStretchLastSection(true);

    tableView->verticalHeader()->hide();
    tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);


    connect(tableView, &QTableView::activated, this, [tableView](const QModelIndex &index){
        if (index.column() == 0)
            tableView->edit(tableView->model()->index(index.row(), 1));
    });


    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tableView);

    QItemEditorFactory *editorFactory = new QItemEditorFactory;
    editorFactory->registerEditor(QMetaType::QColor, new QStandardItemEditorCreator<ColorDialog>());
    editorFactory->registerEditor(QMetaType::QFont, new QStandardItemEditorCreator<FontListEditor>());

    // Create a delgate using the factory
    QStyledItemDelegate *delegate = new QStyledItemDelegate(this);
    delegate->setItemEditorFactory(editorFactory);
    tableView->setItemDelegate(delegate);
}
