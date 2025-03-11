// Copyright (c) 2024 Manuel Schneider

#include "palettereader.h"
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QDir>
#include <QFont>
#include <QMetaEnum>
#include <QPalette>
#include <QSettings>
#include <QStandardPaths>
#include <albert/logging.h>
using namespace std;

template<typename QEnum>
static const char *toString(const QEnum value)
{ return QMetaEnum::fromType<QEnum>().valueToKey(value); }

static QString toString(const QColor &color)
{ return color.name(color.alpha() == 255 ? QColor::HexRgb : QColor::HexArgb); }

static const array<QPalette::ColorGroup, 3> colorGroups
{
    QPalette::Active,
    QPalette::Inactive,
    QPalette::Disabled
};

static const array<QPalette::ColorRole, 14> colorRoles
{
    QPalette::Window,
    QPalette::WindowText,
    QPalette::Button,
    QPalette::ButtonText,
    QPalette::BrightText,
    QPalette::Base,
    QPalette::AlternateBase,
    QPalette::Text,
    QPalette::ToolTipBase,
    QPalette::ToolTipText,
    QPalette::Highlight,
    QPalette::HighlightedText,
    QPalette::Link,
    QPalette::LinkVisited,
    // QPalette::Light,
    // QPalette::Midlight,
    // QPalette::Dark,
    // QPalette::Mid,
    // QPalette::Shadow
};


void writePalette(const QPalette &palette, const QString &path)
{
    QSettings s(path, QSettings::IniFormat);

    if (!s.isWritable())
        throw runtime_error("Path is not writable.");

    for (auto group : colorGroups)
    {
        s.beginGroup(toString(group));
        for (auto role : colorRoles)
            s.setValue(toString(role), toString(palette.color(group, role)));
        s.endGroup();
    }
}

QPalette readPalette(const QString &path)
{
    QSettings s(path, QSettings::IniFormat);

    // QPalette palette;
    // for (auto group : colorGroups)
    // {
    //     s.beginGroup(toString(group));
    //     for (auto role : colorRoles)
    //         if (auto color = s.value(toString(role)).value<QColor>();
    //             color.isValid())
    //             palette.setColor(group, role, color);
    //         else
    //             throw runtime_error("Invalid palette.");
    //     s.endGroup();
    // }

    auto getColor = [&s](QPalette::ColorRole role){
        if (auto color = s.value(toString(role)).value<QColor>();
            color.isValid())
            return color;
        else
            throw runtime_error(string("Invalid palette. Not found: ") + toString(role));
    };


    // QPalette(const QBrush &windowText,
    //          const QBrush &button,
    //          const QBrush &light,
    //          const QBrush &dark,
    //          const QBrush &mid,
    //          const QBrush &text,
    //          const QBrush &bright_text,
    //          const QBrush &base,
    //          const QBrush &window);

    s.beginGroup(toString(QPalette::Active));

    auto Base = getColor(QPalette::Base);
    auto Text = getColor(QPalette::Text);

    auto Window = getColor(QPalette::Window);
    auto WindowText = getColor(QPalette::WindowText);

    auto Button = getColor(QPalette::Button);
    auto ButtonText = getColor(QPalette::ButtonText);

    QColor Light, Mid, Dark;
    try {
        Light = getColor(QPalette::Light);
        Mid = getColor(QPalette::Mid);
        Dark = getColor(QPalette::Dark);
    } catch (...) {
        Light = Button.lighter();
        Mid = Button.darker();
        Dark = Mid.darker();
    }

    QPalette palette(WindowText,
                     Button,
                     Light,
                     Mid,
                     Dark,
                     Text,
                     ButtonText,
                     Base,
                     Window);

    // Mandatory, throws
    palette.setColor(QPalette::All, QPalette::Highlight, getColor(QPalette::Highlight));

    if (auto color = s.value(toString(QPalette::PlaceholderText)).value<QColor>();
        color.isValid())
        palette.setColor(QPalette::All, QPalette::PlaceholderText, color);
    else
        palette.setColor(QPalette::All, QPalette::PlaceholderText, Button);

    if (auto color = s.value(toString(QPalette::HighlightedText)).value<QColor>();
        color.isValid())
        palette.setColor(QPalette::All, QPalette::HighlightedText, color);
    else
        palette.setColor(QPalette::All, QPalette::HighlightedText, ButtonText);

    if (auto color = s.value(toString(QPalette::Link)).value<QColor>();
        color.isValid())
        palette.setColor(QPalette::All, QPalette::Link, color);

    if (auto color = s.value(toString(QPalette::LinkVisited)).value<QColor>();
        color.isValid())
        palette.setColor(QPalette::All, QPalette::LinkVisited, color);


    // QPalette palette;
    // for (auto group : colorGroups)
    // {
    //     s.beginGroup(toString(group));
    //     for (auto role : colorRoles)
    //         if (auto color = s.value(toString(role)).value<QColor>();
    //             color.isValid())
    //             palette.setColor(group, role, color);
    //         else
    //             throw runtime_error("Invalid palette.");
    //     s.endGroup();
    // }

    return palette;
}

map<QString, QString> findPalettes(const QStringList &data_dir_paths)
{
    map<QString, QString> themes;

    for (const auto &data_dir_path : data_dir_paths)
    {
        QDir data_dir(QString("%1/palettes").arg(data_dir_path));
        auto ini_file_infos = data_dir.entryInfoList(QStringList("*.ini"),
                                                     QDir::Files|QDir::NoSymLinks);
        for (const auto &ini_file_info : as_const(ini_file_infos))
            themes.emplace(ini_file_info.baseName(), ini_file_info.canonicalFilePath());
    }

    return themes;
}


// QPalette loadPalette(const QString &path)
// {
//     QPalette pal;
//     QFile f(path);
//     if (f.open(QIODevice::ReadOnly))
//     {
//         QJsonParseError err;
//         auto obj = QJsonDocument::fromJson(f.readAll(), &err).object();
//         f.close();
//         if (err.error != QJsonParseError::NoError)
//         {
//             for (const auto &[k, r] : mapping)
//                 pal.setColor(r, QColor::fromString(obj[k].toString()));
//         }
//         else
//             WARN << "Failed to parse JSON:" << err.errorString();
//     }
//     else
//         WARN << "Failed to open JSON file:" << f.errorString();

//     return pal;
// }

// void savePalette(const QPalette &pal, const QString &path)
// {
//     QJsonObject obj;
//     for (const auto &[key, role] : mapping)
//     {
//         auto c = pal.color(role);
//         obj[key] = c.name(c.alpha() == 255 ? QColor::HexRgb : QColor::HexArgb);
//     }

//     if (QFile f(path); f.open(QIODevice::WriteOnly))
//     {
//         f.write(QJsonDocument(obj).toJson());
//         f.close();
//     }
//     else
//         WARN << "Failed to open JSON file:" << f.errorString();
// }




// Style Style::read(const QString &path)
// {
//     Style style;
//     QSettings s(path, QSettings::IniFormat);

//     s.beginGroup(QStringLiteral("Palette"));
//     QMetaEnum metaEnum = QMetaEnum::fromType<QPalette::ColorRole>();
//     auto palette = style.palette;
//     for (const auto &role : colorRoles)
//     {
//         if (const char *key = metaEnum.valueToKey(role); s.contains(key))
//         {
//             auto value = s.value(key).toString();
//             if (auto color = QColor::fromString(value); color.isValid())
//             {
//                 DEBG << value << color;
//                 palette.setColor(role, color);
//                 continue;
//             } else
//                 WARN << QString("Failed to parse color '%1'.").arg(value);
//         } else
//             WARN << QString("Color unspecified: %1").arg(key);
//     }
//     s.endGroup();

//     style.palette = QPalette(
//         palette.color(QPalette::Button),
//         palette.color(QPalette::Window));



//     #define readValue(val) \
//     style.val = QBrush(s.value(QStringLiteral(#val), style.val).value<QColor>());\
//     DEBG << #val << style.val;

//     s.beginGroup(QStringLiteral("Theme"));
//     readValue(window_background_brush);
//     readValue(window_border_brush);
//     readValue(input_frame_border_brush)
//     readValue(input_frame_background_brush)
//     readValue(item_view_background_brush)
//     readValue(item_view_border_brush)
//     readValue(item_view_item_selection_background_brush)
//     readValue(item_view_item_selection_border_brush)
//     readValue(result_item_text_color)
//     readValue(result_item_subtext_color)
//     readValue(action_item_text_color)
//     s.endGroup();


//     #define readIntValue(val) style.val = s.value(QStringLiteral(#val), style.val).toInt();\
//     s.beginGroup(QStringLiteral("Metrics"));
//     readIntValue(window_border_radius);
//     readIntValue(window_border_width);
//     readIntValue(window_padding);
//     readIntValue(window_width);
//     readIntValue(input_frame_border_radius)
//     readIntValue(input_frame_border_width)
//     readIntValue(input_frame_padding)
//     readIntValue(input_line_font_size)
//     readIntValue(settings_button_color)
//     readIntValue(settings_button_highlight_color)
//     readIntValue(settings_button_size)
//     readIntValue(item_view_border_radius)
//     readIntValue(item_view_border_width)
//     readIntValue(item_view_padding)
//     readIntValue(item_view_item_selection_border_radius)
//     readIntValue(item_view_item_selection_border_width)
//     readIntValue(item_view_item_padding)
//     readIntValue(result_item_icon_size)
//     readIntValue(result_item_text_font_size)
//     readIntValue(result_item_subtext_font_size)
//     readIntValue(result_item_horizontal_spacing)
//     readIntValue(result_item_vertical_spacing)
//     readIntValue(action_item_font_size)
//     s.endGroup();

//     return style;
// }

// bool Style::write(const QString &path) const
// {
//     QSettings s(path, QSettings::IniFormat);

//     if (!s.isWritable()){
//         WARN << "Path not writable:" << path;
//         return false;
//     }

//     s.beginGroup(QStringLiteral("Palette"));
//     for (const auto &[key, role] : mapping)
//         s.setValue(key, toString(palette.color(role)));
//     s.endGroup();

//     #define writeValue(val) s.setValue(QStringLiteral(#val), toString(val));
//     s.beginGroup(QStringLiteral("Style"));
//     writeValue(window_background_brush);
//     writeValue(window_border_brush);
//     writeValue(window_border_radius);
//     writeValue(window_border_width);
//     writeValue(window_padding);
//     writeValue(window_width);
//     writeValue(input_frame_border_brush);
//     writeValue(input_frame_background_brush);
//     writeValue(input_frame_border_radius);
//     writeValue(input_frame_border_width);
//     writeValue(input_frame_padding);
//     writeValue(input_line_font_size);
//     writeValue(settings_button_color);
//     writeValue(settings_button_highlight_color);
//     writeValue(settings_button_size);
//     writeValue(item_view_background_brush);
//     writeValue(item_view_border_brush);
//     writeValue(item_view_border_radius);
//     writeValue(item_view_border_width);
//     writeValue(item_view_padding);
//     writeValue(item_view_item_selection_background_brush);
//     writeValue(item_view_item_selection_border_brush);
//     writeValue(item_view_item_selection_border_radius);
//     writeValue(item_view_item_selection_border_width);
//     writeValue(item_view_item_padding);
//     writeValue(result_item_icon_size);
//     writeValue(result_item_text_color);
//     writeValue(result_item_text_font_size);
//     writeValue(result_item_subtext_color);
//     writeValue(result_item_subtext_font_size);
//     writeValue(result_item_horizontal_spacing);
//     writeValue(result_item_vertical_spacing);
//     writeValue(action_item_text_color);
//     writeValue(action_item_font_size);
//     s.endGroup();

//     return true;
// }

// // QColor Style::parseColor(const QString &)
// // {
// //     QColor c = QColor::fromString(s);
// //     if (!c.isValid())
// //         WARN << "Failed to parse color:" << s;
// // }

// QString Style::toString(const int &number)
// {
//     return QString::number(number);
// }


// QString Style::toString(const QBrush &brush)
// {
//     if (auto img = brush.textureImage(); !img.isNull())
//     {
//         return "img";
//     }
//     else if (auto color = brush.color(); color != QColor::Invalid)
//     {
//         return toString(color);
//     }
//     else if (auto gradient = brush.gradient(); gradient != nullptr)
//     {
//         if (gradient->type() == QGradient::LinearGradient)
//         {

//         }
//         QString str;
//         gradient->stops();

//         for (const QGradientStop &stop : gradient->stops())
//             str += toString(stop.second) + " ";

//         return "some other gradient";
//     }
//     else
//         return QStringLiteral("");
// }
