#include "paletteeditor.h"
#include <QTableView>
#include <QVBoxLayout>

static std::array<QPalette::ColorGroup, 3> colorGroups{
    QPalette::Active,
    QPalette::Inactive,
    QPalette::Disabled
};

static std::array<QPalette::ColorRole, 20> colorRoles{
    QPalette::AlternateBase,
    QPalette::Base,
    QPalette::BrightText,
    QPalette::Button,
    QPalette::ButtonText,
    QPalette::Dark,
    QPalette::Highlight,
    QPalette::HighlightedText,
    QPalette::Light,
    QPalette::Link,
    QPalette::LinkVisited,
    QPalette::Mid,
    QPalette::Midlight,
    QPalette::PlaceholderText,
    QPalette::Shadow,
    QPalette::Text,
    QPalette::ToolTipBase,
    QPalette::ToolTipText,
    QPalette::Window,
    QPalette::WindowText
};

enum ColorRole { WindowText, Button, Light, Midlight, Dark, Mid,
                 Text, BrightText, ButtonText, Base, Window, Shadow,
                 Highlight, HighlightedText,
                 Link, LinkVisited,
                 AlternateBase,
                 NoRole,
                 ToolTipBase, ToolTipText,
                 PlaceholderText,
                 NColorRoles = PlaceholderText + 1,
                 };

class PaletteModel : public QAbstractTableModel
{
public:
    PaletteModel(const QPalette &p, QObject *parent = nullptr):
        QAbstractTableModel(parent), palette(p) {}

    int rowCount(const QModelIndex &) const override
    { return colorRoles.size(); }

    int columnCount(const QModelIndex &) const override
    { return colorGroups.size(); }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (index.isValid() && role == Qt::BackgroundRole)
            return palette.color(colorGroups[index.column()], colorRoles[index.row()]);
        return QVariant();
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role) override
    {
        if (index.isValid() && role == Qt::BackgroundRole){
            palette.setColor(colorGroups[index.column()], colorRoles[index.row()], value.value<QColor>());
            return true;
        }
        return false;
    }

    QPalette palette;
};

PaletteEditor::PaletteEditor(const QPalette &palette, QWidget *parent):
    QDialog(parent)
{
    setLayout(new QVBoxLayout(this));

    QTableView *table_view = new QTableView(this);
    layout()->addWidget(table_view);

    model_ = new PaletteModel(palette);
    table_view->setModel(model_);
}

QPalette PaletteEditor::palette() const
{
    return model_->palette;
}


