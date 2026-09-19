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
#pragma once

#include <QColor>
#include <QPalette>
#include <QString>

#include "ui/themes/ITheme.h"

/**
 * The Star Client application themes: OLED Black, Midnight and Daylight.
 *
 * The stylesheet is not written by hand. theme/tokens.json is the single
 * source of truth and tools/gen_theme.py renders it into
 * src/theme/generated/starclient-<theme>.qss, which is compiled into the
 * binary through src/theme/starclient.qrc. The web prototype is generated
 * from the same tokens, so the two never drift.
 *
 * Qt stylesheets cannot use CSS variables, so the generator writes literal
 * colours for the default accent (starlight). StarTheme re-applies the
 * user's chosen accent at runtime by substituting the colour literals
 * carried in StarThemePalette.h.
 */
class StarTheme : public ITheme {
   public:
    /// Which generated stylesheet this instance wraps.
    enum class Variant { Oled, Midnight, Daylight };

    /// Accent presets, matching theme/tokens.json -> accents.
    enum class Accent { Starlight, Aurora, Nova, Solar, Emerald, Ember };

    explicit StarTheme(Variant variant);

    QString id() override;
    QString name() override;
    QString tooltip() override;
    bool hasStyleSheet() override { return true; }
    QString appStyleSheet() override;
    QString qtTheme() override { return QString(); }
    QPalette colorScheme() override;
    QColor fadeColor() override;
    double fadeAmount() override;
    LogColors logColorScheme() override;

    /// Applies an accent preset. Call before apply(); re-applies if live.
    void setAccent(Accent accent);
    Accent accent() const { return m_accent; }

    /// True when the variant uses a dark palette.
    bool isDark() const;

    /// Human-readable accent name, for the settings UI.
    static QString accentName(Accent accent);
    static QList<Accent> allAccents();
    static QString accentKey(Accent accent);

   private:
    /// Replaces the default accent colours in the generated QSS.
    QString applyAccent(const QString& qss) const;

    /// The accent's colour tuple, replacing a token in the stylesheet.
    struct AccentColors {
        QString accent;
        QString hover;
        QString pressed;
        QString soft;
        QString softStrong;
        QString ring;
        QString gradientFrom;
        QString gradientTo;
        QString rgb;
    };
    static AccentColors colorsFor(Accent accent);

    Variant m_variant;
    Accent m_accent = Accent::Starlight;
};
