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

#include <QSizePolicy>
#include <QPointer>
#include <QStackedWidget>
#include <QString>

#include "ui/StarNavRail.h"

class QLabel;
class QLineEdit;
class QToolButton;
class StarPage;
class QWidget;

/**
 * The content area of the main window: the prototype's title strip (brand,
 * search, quick start, account) over the page stack. One StarPage is the
 * face; transient views -- installing from Browse, editing an instance -- sit
 * on top of it behind a back control.
 *
 * The shell is a QStackedWidget so the launcher sets it as the central
 * widget and gets real navigation without any animation.
 */
class StarShell : public QStackedWidget {
    Q_OBJECT

   public:
    explicit StarShell(QWidget* parent = nullptr);

    void setLauncherPage(QWidget* page);
    QWidget* launcherPage() const;

    /// A page from the nav rail. Owns the launcher's view; returns its face.
    StarPage* showPage(StarNavRail::Page page, QWidget* view, const QString& title, const QString& subtitle = QString());
    StarPage* currentPage() const;
    StarNavRail::Page currentRailPage() const;

    /// Puts a transient view on top of the current page (Install, Edit).
    StarPage* pushSubPage(QWidget* view, const QString& title, const QString& subtitle = QString());
    void popSubPage();
    bool hasSubPage() const;

    /// The title strip. Cursor is parked in the search box; both signals go up.
    QString searchText() const;
    void setSearchText(const QString& text);

    /// The view body of the page on stage (instances grid, browse grid, ...).
    QWidget* currentContent() const;
    void setAccountCaption(const QString& name);

    /// The hero row of the current page: refresh it after the state changes.
    void setPageSubtitle(const QString& subtitle);

   signals:
    void searchChanged(const QString& text);
    /// Enter in the search box: navigate-and-search semantics live upstairs.
    void searchSubmitted(const QString& text);
    void quickStartRequested();
    void accountRequested();
    void subPageBack();

   protected:
    /// Brand and place retranslate with the app.
    void changeEvent(QEvent* event) override;

   private:
    QWidget* buildHeaderBar();
    void retranslate();
    /// Recolours the title-strip star from the active palette.
    void retintBrand();

    StarPage* m_page = nullptr;     // the face shown for the rail's page
    QWidget* m_launcherPage = nullptr;
    QWidget* m_headerBar = nullptr;
    QLineEdit* m_search = nullptr;
    QToolButton* m_quickStart = nullptr;
    QToolButton* m_account = nullptr;
    QLabel* m_brandMark = nullptr;

    QWidget* m_subView = nullptr;   // live sub view, or null
    QPointer<QWidget> m_content;    // the body of the current page
    QString m_accountName;
    QString m_pageTitle;            // remembered so a back restores the title
    QString m_pageSubtitle;
};
