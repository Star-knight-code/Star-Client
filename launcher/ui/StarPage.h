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

#include <QWidget>

#include "ui/StarNavRail.h"

class QHBoxLayout;
class QLabel;
class QToolButton;

/**
 * The face of one page in the main window: a title with a back control over
 * the content. The shell swaps the content but the page is an ordinary widget,
 * so each destination carries its own hero, commands and status in one place.
 *
 * The look is the "surface" step of the design system's elevation, with the
 * title in the prototype's display type and the supporting text underneath.
 */
class StarPage : public QWidget {
    Q_OBJECT

   public:
    explicit StarPage(StarNavRail::Page page, QWidget* parent = nullptr);

    StarNavRail::Page railPage() const { return m_page; }
    void setRailPage(StarNavRail::Page page) { m_page = page; }

    void setTitle(const QString& title);
    QString title() const;
    void setSubtitle(const QString& subtitle);

    /// The widget the page manages (a placeholder when the shell builds it).
    QWidget* body() const { return m_body; }
    void setBody(QWidget* body);

    /// An action button next to the title, e.g. "New instance". Its signals
    /// are the caller's, the look is the design system's.
    QToolButton* addHeaderAction(const QString& iconName, const QString& text, const QString& role = QStringLiteral("primary"));

    /// Whether the back control at the top is shown.
    void setBackVisible(bool visible);
    bool isBackVisible() const;
    void setBackEnabled(bool enabled);
    void setBackToolTip(const QString& text);

   signals:
    /// The back control is pressed. Emitted only when it is enabled.
    void backRequested();

   protected:
    /// Wallpapers repaint when the theme changes.
    void changeEvent(QEvent* event) override;

   private:
    QString displayTitle(StarNavRail::Page page) const;

    StarNavRail::Page m_page;
    QToolButton* m_backButton = nullptr;
    QLabel* m_title = nullptr;
    QLabel* m_subtitle = nullptr;
    QHBoxLayout* m_headerActions = nullptr;
    QWidget* m_body = nullptr;
};
