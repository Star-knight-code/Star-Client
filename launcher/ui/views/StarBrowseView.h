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

#include <QString>
#include <QUrl>
#include <QWidget>

class QGridLayout;
class QLabel;
class QNetworkAccessManager;
class QScrollArea;
class QTimer;
class QToolButton;

/**
 * The Browse destination: a live Modrinth search dressed in the Star Client
 * look. The shell's title-strip search drives it; cards install a modpack in
 * one click (the launcher imports the .mrpack) or open a mod's page.
 */
class StarBrowseView : public QWidget {
    Q_OBJECT

   public:
    explicit StarBrowseView(QWidget* parent = nullptr);

    /// What the search bar contains right now.
    QString lastQuery() const;

   public slots:
    /// Sync from the title-strip search: immediate when empty, debounced when typing.
    void search(const QString& query);
    /// Modpacks or mods: the two sides of the catalogue.
    void setProjectType(const QString& projectType);

   signals:
    /// A modpack's .mrpack download, ready for the import wizard.
    void installRequested(const QUrl& fileUrl, const QString& name);
    /// Anything else (a mod detail page) opens in the browser.
    void openRequested(const QUrl& pageUrl);

   private:
    void performSearch();
    void showResults();
    void requestIcon(class QLabel* target, const QString& url);
    void requestVersionsAndInstall(const QString& projectId, const QString& title, const QString& slug);
    void setStatus(const QString& text);
    void clearResults();

    struct Result {
        QString projectId;
        QString slug;
        QString title;
        QString author;
        QString description;
        QString iconUrl;
        QString category;
        qint64 downloads = 0;
    };
    QList<Result> m_results;

    QNetworkAccessManager* m_nam = nullptr;
    QToolButton* m_modpacksTab = nullptr;
    QToolButton* m_modsTab = nullptr;
    QLabel* m_status = nullptr;
    QScrollArea* m_scroll = nullptr;
    QWidget* m_resultsHost = nullptr;
    QGridLayout* m_grid = nullptr;
    QTimer* m_debounce = nullptr;

    QString m_query;
    QString m_projectType = QStringLiteral("modpack");
    int m_searchSeq = 0;   // stale replies discarded by comparing tokens
};
