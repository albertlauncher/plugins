// Copyright (c) 2024 Manuel Schneider

#include "style.h"
#include <albert/logging.h>

#include <QSettings>
using namespace std;


// QPalette loadPalette(const QString &path)
// {
//     QPalette pal;
//     QSettings s(path, QSettings::IniFormat);
//     s.beginGroup(QStringLiteral("Palette"));

//     for (const auto &[k, r] : mapping)
//         pal.setBrush(r, s.value(k).value<QBrush>());

//     return pal;
// }

// void savePalette(const QPalette &pal, const QString &path)
// {
//     QSettings s(path, QSettings::IniFormat);

//     if (s.isWritable())
//     {
//         s.beginGroup(QStringLiteral("Palette"));
//         for (const auto &[key, role] : mapping)
//         {
//             auto c = pal.color(role);
//             s.setValue(key, c.name(c.alpha() == 255 ? QColor::HexRgb : QColor::HexArgb));



//             // auto img = QImage("/Users/manuel/Desktop/nasa/cool2.png");
//             // s.setValue("test", QBrush(img));
//         }
//     }
//     else
//         WARN << "Cant write palette to" << path;
// }

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


// static const array<QPalette::ColorGroup, 3> colorGroups{
//     QPalette::Active,
//     QPalette::Inactive,
//     QPalette::Disabled
// };

static const array<
    QPalette::ColorRole,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    15
#else
    14
#endif
    > colorRoles{
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
    // only qt above 6.7
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    QPalette::Accent,
#endif
    QPalette::HighlightedText,
    QPalette::Link,
    QPalette::LinkVisited,
    // QPalette::Light,
    // QPalette::Midlight,
    // QPalette::Dark,
    // QPalette::Mid,
    // QPalette::Shadow
};


Style Style::read(const QString &path)
{
    Style style;
    QSettings s(path, QSettings::IniFormat);

    s.beginGroup(QStringLiteral("Palette"));
    QMetaEnum metaEnum = QMetaEnum::fromType<QPalette::ColorRole>();
    auto palette = style.palette;
    for (const auto &role : colorRoles)
    {
        if (const char *key = metaEnum.valueToKey(role); s.contains(key))
        {
            auto value = s.value(key).toString();
            if (auto color = QColor::fromString(value); color.isValid())
            {
                DEBG << value << color;
                palette.setColor(role, color);
                continue;
            } else
                WARN << QString("Failed to parse color '%1'.").arg(value);
        } else
            WARN << QString("Color unspecified: %1").arg(key);
    }
    s.endGroup();

    style.palette = QPalette(
        palette.color(QPalette::Button),
        palette.color(QPalette::Window));



    #define readValue(val) \
    style.val = QBrush(s.value(QStringLiteral(#val), style.val).value<QColor>());\
    DEBG << #val << style.val;

    s.beginGroup(QStringLiteral("Theme"));
    readValue(window_background_brush);
    readValue(window_border_brush);
    readValue(input_frame_border_brush)
    readValue(input_frame_background_brush)
    readValue(item_view_background_brush)
    readValue(item_view_border_brush)
    readValue(item_view_item_selection_background_brush)
    readValue(item_view_item_selection_border_brush)
    readValue(result_item_text_color)
    readValue(result_item_subtext_color)
    readValue(action_item_text_color)
    s.endGroup();


    #define readIntValue(val) style.val = s.value(QStringLiteral(#val), style.val).toInt();\
    s.beginGroup(QStringLiteral("Metrics"));
    readIntValue(window_border_radius);
    readIntValue(window_border_width);
    readIntValue(window_padding);
    readIntValue(window_width);
    readIntValue(input_frame_border_radius)
    readIntValue(input_frame_border_width)
    readIntValue(input_frame_padding)
    readIntValue(input_line_font_size)
    readIntValue(settings_button_color)
    readIntValue(settings_button_highlight_color)
    readIntValue(settings_button_size)
    readIntValue(item_view_border_radius)
    readIntValue(item_view_border_width)
    readIntValue(item_view_padding)
    readIntValue(item_view_item_selection_border_radius)
    readIntValue(item_view_item_selection_border_width)
    readIntValue(item_view_item_padding)
    readIntValue(result_item_icon_size)
    readIntValue(result_item_text_font_size)
    readIntValue(result_item_subtext_font_size)
    readIntValue(result_item_horizontal_spacing)
    readIntValue(result_item_vertical_spacing)
    readIntValue(action_item_font_size)
    s.endGroup();

    return style;
}

bool Style::write(const QString &path) const
{
    QSettings s(path, QSettings::IniFormat);

    if (!s.isWritable()){
        WARN << "Path not writable:" << path;
        return false;
    }

    s.beginGroup(QStringLiteral("Palette"));
    for (const auto &[key, role] : mapping)
        s.setValue(key, toString(palette.color(role)));
    s.endGroup();

    #define writeValue(val) s.setValue(QStringLiteral(#val), toString(val));
    s.beginGroup(QStringLiteral("Style"));
    writeValue(window_background_brush);
    writeValue(window_border_brush);
    writeValue(window_border_radius);
    writeValue(window_border_width);
    writeValue(window_padding);
    writeValue(window_width);
    writeValue(input_frame_border_brush);
    writeValue(input_frame_background_brush);
    writeValue(input_frame_border_radius);
    writeValue(input_frame_border_width);
    writeValue(input_frame_padding);
    writeValue(input_line_font_size);
    writeValue(settings_button_color);
    writeValue(settings_button_highlight_color);
    writeValue(settings_button_size);
    writeValue(item_view_background_brush);
    writeValue(item_view_border_brush);
    writeValue(item_view_border_radius);
    writeValue(item_view_border_width);
    writeValue(item_view_padding);
    writeValue(item_view_item_selection_background_brush);
    writeValue(item_view_item_selection_border_brush);
    writeValue(item_view_item_selection_border_radius);
    writeValue(item_view_item_selection_border_width);
    writeValue(item_view_item_padding);
    writeValue(result_item_icon_size);
    writeValue(result_item_text_color);
    writeValue(result_item_text_font_size);
    writeValue(result_item_subtext_color);
    writeValue(result_item_subtext_font_size);
    writeValue(result_item_horizontal_spacing);
    writeValue(result_item_vertical_spacing);
    writeValue(action_item_text_color);
    writeValue(action_item_font_size);
    s.endGroup();

    return true;
}

// QColor Style::parseColor(const QString &)
// {
//     QColor c = QColor::fromString(s);
//     if (!c.isValid())
//         WARN << "Failed to parse color:" << s;
// }

QString Style::toString(const int &number)
{
    return QString::number(number);
}

QString Style::toString(const QColor &color)
{
    return color.name(color.alpha() == 255 ? QColor::HexRgb : QColor::HexArgb);
}

QString Style::toString(const QBrush &brush)
{
    if (auto img = brush.textureImage(); !img.isNull())
    {
        return "img";
    }
    else if (auto color = brush.color(); color != QColor::Invalid)
    {
        return toString(color);
    }
    else if (auto gradient = brush.gradient(); gradient != nullptr)
    {
        if (gradient->type() == QGradient::LinearGradient)
        {

        }
        QString str;
        gradient->stops();

        for (const QGradientStop &stop : gradient->stops())
            str += toString(stop.second) + " ";

        return "some other gradient";
    }
    else
        return QStringLiteral("");
}
