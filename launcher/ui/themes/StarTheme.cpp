// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Star Client - Minecraft Launcher
 *  Copyright (C) 2026 Star Client Contributors
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "StarTheme.h"

#include <QFile>
#include <QObject>

#include "Application.h"
#include "settings/SettingsObject.h"

namespace {

/// Path inside starclient.qrc for a variant.
QString qssPathFor(StarTheme::Variant variant)
{
    switch (variant) {
        case StarTheme::Variant::Oled:
            return ":/starclient/generated/starclient-oled.qss";
        case StarTheme::Variant::Midnight:
            return ":/starclient/generated/starclient-dark.qss";
        case StarTheme::Variant::Navy:
            return ":/starclient/generated/starclient-navy.qss";
        case StarTheme::Variant::Daylight:
            return ":/starclient/generated/starclient-light.qss";
        case StarTheme::Variant::White:
            return ":/starclient/generated/starclient-white.qss";
    }
    return {};
}

QString idFor(StarTheme::Variant variant)
{
    switch (variant) {
        case StarTheme::Variant::Oled:
            return "star-oled";
        case StarTheme::Variant::Midnight:
            return "star-midnight";
        case StarTheme::Variant::Navy:
            return "star-navy";
        case StarTheme::Variant::Daylight:
            return "star-daylight";
        case StarTheme::Variant::White:
            return "star-white";
    }
    return {};
}

}  // namespace

StarTheme::StarTheme(Variant variant) : m_variant(variant)
{
    // Start on whatever accent the user last picked, if it is still valid.
    const QString saved = APPLICATION->settings()->get("StarAccent").toString();
    for (auto accent : allAccents()) {
        if (accentKey(accent) == saved) {
            m_accent = accent;
            break;
        }
    }
}

QString StarTheme::id()
{
    return idFor(m_variant);
}

QString StarTheme::name()
{
    switch (m_variant) {
        case Variant::Oled:
            return QObject::tr("Star Client OLED Black", "Application theme name");
        case Variant::Midnight:
            return QObject::tr("Star Client Midnight", "Application theme name");
        case Variant::Navy:
            return QObject::tr("Star Client Navy", "Application theme name");
        case Variant::Daylight:
            return QObject::tr("Star Client Daylight", "Application theme name");
        case Variant::White:
            return QObject::tr("Star Client White", "Application theme name");
    }
    return {};
}

QString StarTheme::tooltip()
{
    switch (m_variant) {
        case Variant::Oled:
            return QObject::tr("True #000000 panels. The signature Star Client look, easiest on OLED displays.");
        case Variant::Midnight:
            return QObject::tr("Soft dark greys with lower contrast than OLED Black.");
        case Variant::Navy:
            return QObject::tr("Midnight's dark greys tinted deep blue.");
        case Variant::Daylight:
            return QObject::tr("Bright, high-contrast light theme for daytime use.");
        case Variant::White:
            return QObject::tr("Pure white light theme - the cleanest variant.");
    }
    return {};
}

bool StarTheme::isDark() const
{
    return m_variant == Variant::Oled || m_variant == Variant::Midnight || m_variant == Variant::Navy;
}

QString StarTheme::appStyleSheet()
{
    QFile file(qssPathFor(m_variant));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[Theme] Could not open Star Client stylesheet" << qssPathFor(m_variant)
                   << "- was src/theme/starclient.qrc compiled in?";
        return {};
    }
    const QString qss = QString::fromUtf8(file.readAll());
    return applyAccent(qss);
}

void StarTheme::setAccent(Accent accent)
{
    m_accent = accent;
    APPLICATION->settings()->set("StarAccent", accentKey(accent));
}

QString StarTheme::accentKey(Accent accent)
{
    switch (accent) {
        case Accent::Starlight:
            return "starlight";
        case Accent::Aurora:
            return "aurora";
        case Accent::Nova:
            return "nova";
        case Accent::Solar:
            return "solar";
        case Accent::Emerald:
            return "emerald";
        case Accent::Ember:
            return "ember";
    }
    return "starlight";
}

QString StarTheme::accentName(Accent accent)
{
    switch (accent) {
        case Accent::Starlight:
            return QObject::tr("Starlight");
        case Accent::Aurora:
            return QObject::tr("Aurora");
        case Accent::Nova:
            return QObject::tr("Nova");
        case Accent::Solar:
            return QObject::tr("Solar");
        case Accent::Emerald:
            return QObject::tr("Emerald");
        case Accent::Ember:
            return QObject::tr("Ember");
    }
    return {};
}

QList<StarTheme::Accent> StarTheme::allAccents()
{
    return { Accent::Starlight, Accent::Aurora, Accent::Nova, Accent::Solar, Accent::Emerald, Accent::Ember };
}

/// Values mirror theme/tokens.json -> accents. Starlight is the colour the
/// generator baked into the .qss, so it doubles as the search key.
StarTheme::AccentColors StarTheme::colorsFor(Accent accent)
{
    switch (accent) {
        case Accent::Starlight:
            return { "#7C6BFF", "#9083FF", "#6450F0", "rgba(124,107,255,0.14)", "rgba(124,107,255,0.24)",
                     "rgba(124,107,255,0.45)", "#8B7BFF", "#4FC3F7", "124,107,255" };
        case Accent::Aurora:
            return { "#35D6ED", "#5FE3F5", "#1FBAD2", "rgba(53,214,237,0.14)", "rgba(53,214,237,0.24)",
                     "rgba(53,214,237,0.45)", "#35D6ED", "#7C6BFF", "53,214,237" };
        case Accent::Nova:
            return { "#FF5FA2", "#FF7DB6", "#EC4A8D", "rgba(255,95,162,0.14)", "rgba(255,95,162,0.24)",
                     "rgba(255,95,162,0.45)", "#FF7DB6", "#8B7BFF", "255,95,162" };
        case Accent::Solar:
            return { "#FFB63D", "#FFC862", "#EFA227", "rgba(255,182,61,0.14)", "rgba(255,182,61,0.24)",
                     "rgba(255,182,61,0.45)", "#FFD06A", "#FF8A3D", "255,182,61" };
        case Accent::Emerald:
            return { "#3ED598", "#5FE2AC", "#2BBF83", "rgba(62,213,152,0.14)", "rgba(62,213,152,0.24)",
                     "rgba(62,213,152,0.45)", "#5FE2AC", "#35D6ED", "62,213,152" };
        case Accent::Ember:
            return { "#FF6B4A", "#FF8468", "#EA5636", "rgba(255,107,74,0.14)", "rgba(255,107,74,0.24)",
                     "rgba(255,107,74,0.45)", "#FF9A6A", "#FF5FA2", "255,107,74" };
    }
    return colorsFor(Accent::Starlight);
}

QString StarTheme::applyAccent(const QString& qss) const
{
    const AccentColors target = colorsFor(m_accent);
    const AccentColors base = colorsFor(Accent::Starlight);
    if (target.accent == base.accent) {
        return qss;  // the generated stylesheet already uses this accent
    }

    QString out = qss;
    // Longest strings first: the rgba() forms contain the hex-free numbers, and
    // replacing a short hex before a longer rgba could corrupt the rgba's digits.
    const QList<QPair<QString, QString>> swaps = {
        { base.softStrong, target.softStrong }, { base.soft, target.soft }, { base.ring, target.ring },
        { base.hover, target.hover },           { base.pressed, target.pressed },
        { base.gradientFrom, target.gradientFrom }, { base.gradientTo, target.gradientTo },
        { base.accent, target.accent },
    };
    for (const auto& [from, to] : swaps) {
        out.replace(from, to);
    }
    return out;
}

QPalette StarTheme::colorScheme()
{
    // Base palette for widgets Qt paints itself (native scroll areas, tooltips
    // and the like). The stylesheet handles everything we control directly.
    QPalette palette;
    if (isDark()) {
        const bool oled = m_variant == Variant::Oled;
        const QColor window = oled ? QColor("#000000") : QColor("#0C0C11");
        const QColor base = oled ? QColor("#0A0A0E") : QColor("#101017");
        const QColor text = oled ? QColor("#F3F4F8") : QColor("#EEF0F6");

        palette.setColor(QPalette::Window, window);
        palette.setColor(QPalette::WindowText, text);
        palette.setColor(QPalette::Base, base);
        palette.setColor(QPalette::AlternateBase, oled ? QColor("#07070A") : QColor("#15151C"));
        palette.setColor(QPalette::ToolTipBase, oled ? QColor("#121218") : QColor("#1B1B23"));
        palette.setColor(QPalette::ToolTipText, text);
        palette.setColor(QPalette::Text, text);
        palette.setColor(QPalette::Button, oled ? QColor("#0B0B0F") : QColor("#15151C"));
        palette.setColor(QPalette::ButtonText, text);
        palette.setColor(QPalette::BrightText, QColor("#FF5C6C"));
        palette.setColor(QPalette::Link, QColor(colorsFor(m_accent).accent));
        palette.setColor(QPalette::Highlight, QColor(colorsFor(m_accent).accent));
        palette.setColor(QPalette::HighlightedText, Qt::white);
        palette.setColor(QPalette::PlaceholderText, oled ? QColor("#6C7080") : QColor("#71758A"));
        palette.setColor(QPalette::Disabled, QPalette::Text, oled ? QColor("#4A4E5C") : QColor("#4E5260"));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, oled ? QColor("#4A4E5C") : QColor("#4E5260"));
    } else {
        palette.setColor(QPalette::Window, QColor("#F2F4FA"));
        palette.setColor(QPalette::WindowText, QColor("#0F131C"));
        palette.setColor(QPalette::Base, QColor("#FFFFFF"));
        palette.setColor(QPalette::AlternateBase, QColor("#F7F8FC"));
        palette.setColor(QPalette::ToolTipBase, QColor("#FFFFFF"));
        palette.setColor(QPalette::ToolTipText, QColor("#0F131C"));
        palette.setColor(QPalette::Text, QColor("#0F131C"));
        palette.setColor(QPalette::Button, QColor("#FFFFFF"));
        palette.setColor(QPalette::ButtonText, QColor("#0F131C"));
        palette.setColor(QPalette::BrightText, QColor("#D93A4C"));
        palette.setColor(QPalette::Link, QColor("#6450F0"));
        palette.setColor(QPalette::Highlight, QColor("#6450F0"));
        palette.setColor(QPalette::HighlightedText, Qt::white);
        palette.setColor(QPalette::PlaceholderText, QColor("#767C90"));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#A7ADBE"));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#A7ADBE"));
    }
    return fadeInactive(palette, fadeAmount(), fadeColor());
}

QColor StarTheme::fadeColor()
{
    return isDark() ? Qt::black : Qt::white;
}

double StarTheme::fadeAmount()
{
    return 0.0;  // the Star Client palettes are already flat and intentional
}

LogColors StarTheme::logColorScheme()
{
    // Log view colours from the theme tokens, so warnings and errors match the
    // rest of the interface instead of the generic palette.
    LogColors colors;
    const bool dark = isDark();
    colors.background[MessageLevel::Launcher] = dark ? QColor("#07070A") : QColor("#F7F8FC");
    colors.background[MessageLevel::Info] = QColor("#0B0B0F");
    colors.background[MessageLevel::Warning] = dark ? QColor("#FFC24B") : QColor("#B47500");
    colors.background[MessageLevel::Error] = dark ? QColor("#FF5C6C") : QColor("#D93A4C");
    colors.background[MessageLevel::Fatal] = dark ? QColor("#FF5C6C") : QColor("#D93A4C");
    colors.background[MessageLevel::Debug] = dark ? QColor("#4CC2FF") : QColor("#1273C4");

    colors.foreground[MessageLevel::Launcher] = dark ? QColor("#F3F4F8") : QColor("#0F131C");
    colors.foreground[MessageLevel::Info] = dark ? QColor("#A6AAB8") : QColor("#4B5163");
    colors.foreground[MessageLevel::Warning] = QColor("#1A1A1A");
    colors.foreground[MessageLevel::Error] = QColor("#1A1A1A");
    colors.foreground[MessageLevel::Fatal] = QColor("#FFFFFF");
    colors.foreground[MessageLevel::Debug] = QColor("#1A1A1A");
    return colors;
}
