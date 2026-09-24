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
#include "StarBrowseView.h"

#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPalette>
#include <QPixmap>
#include <QPointer>
#include <QScrollArea>
#include <QSizePolicy>
#include <QTimer>
#include <QToolButton>
#include <QUrlQuery>
#include <QVBoxLayout>

namespace {

constexpr int kResultLimit = 24;
constexpr int kIconSize = 48;

QNetworkRequest modrinthRequest(const QUrl& url)
{
    QNetworkRequest request(url);
    // the catalogue's house rule: identify yourself and where you came from
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("Star-knight-code/Star-Client/0.1 (https://github.com/Star-knight-code/Star-Client)"));
    return request;
}

QString shortDownloads(qint64 value)
{
    if (value >= 1000000) {
        return QStringLiteral("%1M").arg(value / 1000000.0, 0, 'f', 1);
    }
    if (value >= 1000) {
        return QStringLiteral("%1k").arg(value / 1000);
    }
    return QString::number(value);
}

}  // namespace

StarBrowseView::StarBrowseView(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_nam = new QNetworkAccessManager(this);

    auto* column = new QVBoxLayout(this);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(0);

    // type switch: the catalogue split, as two pills under the page header
    auto* tabs = new QHBoxLayout();
    tabs->setContentsMargins(0, 0, 0, 10);
    tabs->setSpacing(10);

    auto* makeTab = [this](const QString& text) -> QToolButton* {
        auto* button = new QToolButton(this);
        button->setProperty("starRole", QStringLiteral("pill"));
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setText(text);
        button->setToolTip(text);
        return button;
    };
    m_modpacksTab = makeTab(tr("Modpacks"));
    m_modsTab = makeTab(tr("Mods"));
    m_modpacksTab->setChecked(true);

    tabs->addWidget(m_modpacksTab);
    tabs->addWidget(m_modsTab);
    tabs->addStretch(1);
    column->addLayout(tabs);

    connect(m_modpacksTab, &QToolButton::clicked, this, [this] { setProjectType(QStringLiteral("modpack")); });
    connect(m_modsTab, &QToolButton::clicked, this, [this] { setProjectType(QStringLiteral("mod")); });

    m_status = new QLabel(tr("Popular on Modrinth right now."), this);
    m_status->setProperty("starRole", QStringLiteral("muted"));
    m_status->setContentsMargins(0, 0, 0, 8);
    column->addWidget(m_status);

    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_resultsHost = new QWidget();
    m_grid = new QGridLayout(m_resultsHost);
    m_grid->setContentsMargins(0, 0, 6, 0);
    m_grid->setSpacing(12);
    m_scroll->setWidget(m_resultsHost);
    column->addWidget(m_scroll, 1);

    m_debounce = new QTimer(this);
    m_debounce->setSingleShot(true);
    m_debounce->setInterval(450);
    connect(m_debounce, &QTimer::timeout, this, &StarBrowseView::performSearch);

    search(QString());
}

QString StarBrowseView::lastQuery() const
{
    return m_query;
}

void StarBrowseView::search(const QString& query)
{
    m_query = query.trimmed();
    m_debounce->start();
}

void StarBrowseView::setProjectType(const QString& projectType)
{
    if (m_projectType == projectType) {
        return;
    }
    m_projectType = projectType;
    m_modpacksTab->setChecked(projectType == QLatin1String("modpack"));
    m_modsTab->setChecked(projectType == QLatin1String("mod"));
    performSearch();
}

void StarBrowseView::performSearch()
{
    m_results.clear();
    clearResults();
    setStatus(m_query.isEmpty() ? tr("Popular on Modrinth right now.") : tr("Searching Modrinth for %1 ...").arg(m_query));

    QUrl url(QStringLiteral("https://api.modrinth.com/v2/search"));
    QUrlQuery params;
    params.addQueryItem(QStringLiteral("limit"), QString::number(kResultLimit));
    params.addQueryItem(QStringLiteral("index"), QStringLiteral("relevance"));
    if (!m_query.isEmpty()) {
        params.addQueryItem(QStringLiteral("query"), m_query);
    }
    params.addQueryItem(QStringLiteral("facets"),
                        QStringLiteral("[[\"project_type:%1\"]]").arg(m_projectType));
    url.setQuery(params);

    const int token = ++m_searchSeq;
    QNetworkReply* reply = m_nam->get(modrinthRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, token] {
        reply->deleteLater();
        if (token != m_searchSeq) {
            return;  // a newer search already owns the grid
        }
        if (reply->error() != QNetworkReply::NoError) {
            setStatus(tr("Could not reach Modrinth. Check your connection and try again."));
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonArray hits = doc.object().value(QStringLiteral("hits")).toArray();
        m_results.reserve(hits.size());
        for (const QJsonValue& hit : hits) {
            const QJsonObject object = hit.toObject();
            Result result;
            result.projectId = object.value(QStringLiteral("project_id")).toString();
            result.slug = object.value(QStringLiteral("slug")).toString();
            result.title = object.value(QStringLiteral("title")).toString();
            result.author = object.value(QStringLiteral("author")).toString();
            result.description = object.value(QStringLiteral("description")).toString();
            result.iconUrl = object.value(QStringLiteral("icon_url")).toString();
            result.downloads = object.value(QStringLiteral("downloads")).toVariant().toLongLong();
            const QJsonArray categories = object.value(QStringLiteral("categories")).toArray();
            if (!categories.isEmpty()) {
                result.category = categories.first().toString();
            }
            m_results.append(result);
        }
        showResults();
    });
}

void StarBrowseView::clearResults()
{
    while (QLayoutItem* item = m_grid->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

void StarBrowseView::showResults()
{
    clearResults();

    if (m_results.isEmpty()) {
        setStatus(tr("Nothing on Modrinth matches %1.").arg(m_query));
        return;
    }
    setStatus(m_query.isEmpty() ? tr("Popular on Modrinth right now.")
                                : tr("%1 result(s) on Modrinth for %2").arg(m_results.size()).arg(m_query));

    const bool modpacks = m_projectType == QLatin1String("modpack");
    int columnCount = 2;
    for (int i = 0; i < m_results.size(); ++i) {
        const Result& result = m_results.at(i);

        auto* card = new QWidget(m_resultsHost);
        card->setProperty("starRole", QStringLiteral("card"));
        card->setMinimumHeight(150);

        auto* cardColumn = new QVBoxLayout(card);
        cardColumn->setContentsMargins(16, 14, 16, 14);
        cardColumn->setSpacing(6);

        auto* topRow = new QHBoxLayout();
        topRow->setSpacing(12);

        auto* iconLabel = new QLabel(card);
        iconLabel->setFixedSize(kIconSize, kIconSize);
        iconLabel->setProperty("starRole", QStringLiteral("sunken"));
        iconLabel->setAlignment(Qt::AlignCenter);
        topRow->addWidget(iconLabel, 0, Qt::AlignTop);

        auto* titleColumn = new QVBoxLayout();
        titleColumn->setSpacing(2);

        auto* title = new QLabel(result.title, card);
        title->setProperty("starRole", QStringLiteral("h2"));
        titleColumn->addWidget(title);

        QString byline = result.author.isEmpty() ? result.category : tr("by %1 \u00b7 %2").arg(result.author, result.category);
        auto* author = new QLabel(byline, card);
        author->setProperty("starRole", QStringLiteral("muted"));
        titleColumn->addWidget(author);

        titleColumn->addStretch(1);
        topRow->addLayout(titleColumn, 1);
        cardColumn->addLayout(topRow);

        auto* description = new QLabel(result.description, card);
        description->setProperty("starRole", QStringLiteral("muted"));
        description->setWordWrap(true);
        description->setMaximumHeight(fontMetrics().lineSpacing() * 2 + 6);
        cardColumn->addWidget(description);

        auto* bottomRow = new QHBoxLayout();
        bottomRow->setSpacing(10);

        auto* downloads = new QLabel(tr("%1 downloads").arg(shortDownloads(result.downloads)), card);
        downloads->setProperty("starRole", QStringLiteral("caption"));
        bottomRow->addWidget(downloads);
        bottomRow->addStretch(1);

        auto* action = new QToolButton(card);
        action->setCursor(Qt::PointingHandCursor);
        action->setToolButtonStyle(Qt::ToolButtonTextOnly);
        const QString projectUrl = QStringLiteral("https://modrinth.com/%1/%2").arg(m_projectType, result.slug);
        if (modpacks) {
            action->setProperty("starRole", QStringLiteral("primary"));
            action->setText(tr("Install"));
            const QString id = result.projectId;
            const QString name = result.title;
            const QString slug = result.slug;
            connect(action, &QToolButton::clicked, this,
                    [this, id, name, slug] { requestVersionsAndInstall(id, name, slug); });
        } else {
            action->setProperty("starRole", QStringLiteral("outline"));
            action->setText(tr("Open"));
            connect(action, &QToolButton::clicked, this,
                    [this, projectUrl] { emit openRequested(projectUrl); });
        }
        bottomRow->addWidget(action);

        cardColumn->addLayout(bottomRow);

        m_grid->addWidget(card, i / columnCount, i % columnCount);

        if (!result.iconUrl.isEmpty()) {
            requestIcon(iconLabel, result.iconUrl);
        }
    }
    m_grid->setRowStretch(m_results.size() / 2 + 1, 1);
}

void StarBrowseView::requestIcon(QLabel* target, const QString& url)
{
    QPointer<QLabel> guard = target;
    QNetworkReply* reply = m_nam->get(modrinthRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, [guard, reply] {
        reply->deleteLater();
        if (!guard || reply->error() != QNetworkReply::NoError) {
            return;
        }
        QPixmap pixmap;
        pixmap.loadFromData(reply->readAll());
        if (!pixmap.isNull()) {
            guard->setPixmap(pixmap.scaled(QSize(kIconSize, kIconSize), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    });
}

void StarBrowseView::requestVersionsAndInstall(const QString& projectId, const QString& title, const QString& slug)
{
    setStatus(tr("Finding the latest %1 package ...").arg(title));
    const QUrl url(QStringLiteral("https://api.modrinth.com/v2/project/%1/versions").arg(projectId));
    QNetworkReply* reply = m_nam->get(modrinthRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, title, slug] {
        reply->deleteLater();
        const QString pageUrl = QStringLiteral("https://modrinth.com/modpack/%1").arg(slug);
        auto openPage = [this, pageUrl] {
            setStatus(tr("Opening %1 instead.").arg(pageUrl));
            emit openRequested(pageUrl);
        };
        if (reply->error() != QNetworkReply::NoError) {
            openPage();
            return;
        }
        const QJsonArray versions = QJsonDocument::fromJson(reply->readAll()).array();
        for (const QJsonValue& version : versions) {
            const QJsonArray files = version.toObject().value(QStringLiteral("files")).toArray();
            for (const QJsonValue& entry : files) {
                const QJsonObject file = entry.toObject();
                const QString fileUrl = file.value(QStringLiteral("url")).toString();
                if (fileUrl.endsWith(QLatin1String(".mrpack"))) {
                    setStatus(tr("Handing %1 to the installer ...").arg(title));
                    emit installRequested(QUrl(fileUrl), title);
                    return;
                }
            }
        }
        openPage();
    });
}

void StarBrowseView::setStatus(const QString& text)
{
    if (m_status) {
        m_status->setText(text);
    }
}
