// Copyright (c) 2024-2025 Manuel Schneider

#include "paletteeditor.h"
#include <QTableView>
#include <QVBoxLayout>
#include <QMetaEnum>
#include <QHeaderView>
#include <QStyleFactory>

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

static QString toString(const QColor &color)
{ return color.name(color.alpha() == 255 ? QColor::HexRgb : QColor::HexArgb); }



template<typename QEnum>
static const char *toString(const QEnum value)
{ return QMetaEnum::fromType<QEnum>().valueToKey(value); }


class PaletteModel : public QAbstractTableModel
{
public:
    PaletteModel(const QPalette &p, QObject *parent = nullptr):
        QAbstractTableModel(parent), palette(p) {}

    int rowCount(const QModelIndex &) const override
    { return colorRoles.size(); }

    int columnCount(const QModelIndex &) const override
    { return colorGroups.size(); }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (role == Qt::DisplayRole) {
            if (orientation == Qt::Horizontal) {
                return toString(colorGroups[section]);
            } else if (orientation == Qt::Vertical) {
                return toString(colorRoles[section]);
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid())
            return {};

        auto c = palette.color(colorGroups[index.column()], colorRoles[index.row()]);

        if (role == Qt::BackgroundRole)
            return c;

        if (role == Qt::DisplayRole)
            return toString(c);

        if (role == Qt::ForegroundRole)
        {
            c.setRed(255 - c.red());
            c.setBlue(255 - c.blue());
            c.setGreen(255 - c.green());
            c.setAlpha(255);
            return c;
        }

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

static void setStyleRecursive(QWidget *widget, QStyle *style)
{
    widget->setStyle(style);
    for (auto child : widget->findChildren<QWidget*>())
        setStyleRecursive(child, style);
}

PaletteEditor::PaletteEditor(const QPalette &palette, QWidget *parent):
    QDialog(parent)
{
    QTableView *table_view = new QTableView(this);
    setLayout(new QVBoxLayout(this));
    layout()->addWidget(table_view);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    auto *style = QStyleFactory::create("Fusion");
    style->setParent(this);
    setStyleRecursive(this, style);
    setPalette(palette);

    table_view->setModel(new PaletteModel(palette,this));

    int width = table_view->verticalHeader()->width();
    for (int col = 0; col < table_view->model()->columnCount(); ++col)
        width += table_view->columnWidth(col);
    width += table_view->frameWidth() * 2;
    int height = table_view->horizontalHeader()->height();
    for (int row = 0; row < table_view->model()->rowCount(); ++row)
        height += table_view->rowHeight(row);
    height += table_view->frameWidth() * 2;
    table_view->setFixedSize(width, height);
}

QPalette PaletteEditor::palette() const
{
    return model_->palette;
}


