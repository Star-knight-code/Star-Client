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

#include <QIcon>
#include <QList>
#include <QString>
#include <QToolBar>

class QAction;
class QActionGroup;
class QEvent;
class QToolButton;

/**
 * The navigation rail down the left edge: the star mark, the top-level
 * destinations, and the appearance switch above the settings entry.
 *
 * It mirrors the rail in the Star Client prototype. The glyphs are drawn from
 * the design system's SVGs and re-tinted from the active palette whenever the
 * theme changes, because a Qt stylesheet cannot recolour an icon.
 */
class StarNavRail : public QToolBar {
    Q_OBJECT

   public:
    /// The destinations the rail can point at.
    enum class Page { Start, Instances, Browse, Accounts, Settings };

    explicit StarNavRail(QWidget* parent = nullptr);

    /// Marks a destination as the current one. Panes the rail does not own,
    /// such as the instance toolbar, are cleared.
    void setCurrentPage(Page page);
    Page currentPage() const { return m_currentPage; }

   signals:
    void startRequested();
    void instancesRequested();
    void browseRequested();
    void accountsRequested();
    void settingsRequested();
    void appearanceToggleRequested();

   protected:
    /// Re-tints the glyphs when the palette, the language or the DPI changes.
    void changeEvent(QEvent* event) override;

   private:
    void buildActions();
    void retint();
    void retranslate();

    /// Adds one destination button and returns its action.
    QAction* addPageAction(Page page, const QString& iconName);
    /// Adds the bottom group, which is pushed down by a stretch.
    void addBottomActions();
    /// The button the QSS styles as a rail entry.
    static void dressButton(QToolButton* button);

    /// An icon whose idle, hover and selected pixels come from the palette.
    static QIcon railIcon(const QString& resourcePath, int size);
    /// The four-pointed star mark, filled with the theme's star colour.
    static QIcon starMarkIcon(int size);
    /// Localised name of a destination.
    static QString pageTitle(Page page);

    QActionGroup* m_pages = nullptr;
    QList<QAction*> m_pageActions;
    QAction* m_settingsAction = nullptr;
    QAction* m_appearanceAction = nullptr;
    QAction* m_starAction = nullptr;

    Page m_currentPage = Page::Start;
};
