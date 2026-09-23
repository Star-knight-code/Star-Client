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
#include "StarShell.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QPainter>
#include <QSizePolicy>
#include <QToolButton>
#include <QVBoxLayout>

#include "StarPage.h"

StarShell::StarShell(QWidget* parent) : QStackedWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

QWidget* StarShell::buildHeaderBar()
{
    m_headerBar = new QWidget();
    m_headerBar->setObjectName(QStringLiteral("starHeader"));

    auto* row = new QHBoxLayout(m_headerBar);
    row->setContentsMargins(24, 14, 24, 14);
    row->setSpacing(12);

    // brand: the mark in the accent, with the launcher's name lit dark
    auto* brand = new QWidget(m_headerBar);
    brand->setCursor(Qt::PointingHandCursor);
    auto* brandRow = new QHBoxLayout(brand);
    brandRow->setContentsMargins(0, 0, 0, 0);
    brandRow->setSpacing(9);

    m_brandMark = new QLabel(brand);
    brandRow->addWidget(m_brandMark);
    retintBrand();

    auto* name = new QLabel(QStringLiteral("Star Client"), brand);
    name->setProperty("starRole", QStringLiteral("h2"));
    brandRow->addWidget(name);
    row->addWidget(brand);

    row->addSpacing(18);

    m_search = new QLineEdit(m_headerBar);
    m_search->setObjectName(QStringLiteral("starSearch"));
    m_search->setProperty("starRole", QStringLiteral("search"));
    m_search->setClearButtonEnabled(true);
    m_search->setPlaceholderText(tr("Search instances, worlds, mods"));
    connect(m_search, &QLineEdit::textChanged, this, &StarShell::searchChanged);
    row->addWidget(m_search, 1);

    m_quickStart = new QToolButton(m_headerBar);
    m_quickStart->setObjectName(QStringLiteral("starQuickStart"));
    m_quickStart->setProperty("starRole", QStringLiteral("primary"));
    m_quickStart->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_quickStart->setCursor(Qt::PointingHandCursor);
    m_quickStart->setToolTip(tr("Launch the last played instance"));
    connect(m_quickStart, &QToolButton::clicked, this, &StarShell::quickStartRequested);
    row->addWidget(m_quickStart);

    m_account = new QToolButton(m_headerBar);
    m_account->setObjectName(QStringLiteral("starAccount"));
    m_account->setProperty("starRole", QStringLiteral("outline"));
    m_account->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_account->setCursor(Qt::PointingHandCursor);
    connect(m_account, &QToolButton::clicked, this, &StarShell::accountRequested);
    row->addWidget(m_account);

    return m_headerBar;
}

void StarShell::setLauncherPage(QWidget* page)
{
    m_launcherPage = page;
}

QWidget* StarShell::launcherPage() const
{
    return m_launcherPage;
}

StarPage* StarShell::showPage(StarNavRail::Page page, QWidget* view, const QString& title, const QString& subtitle)
{
    if (m_subView) {
        // a rail move out from under a transient view dismisses it
        popSubPage();
    }
    if (m_page) {
        // The launcher's view is a permanent citizen of the window: it was
        // parented into the page about to be destroyed, so unhook it first or
        // the next rail move crashes on a dangling pointer.
        if (m_launcherPage) {
            m_launcherPage->setParent(this);
            m_launcherPage->setVisible(false);
        }
        removeWidget(m_page);
        m_page->deleteLater();
    }

    m_page = new StarPage(page);
    m_page->setTitle(title);
    m_page->setSubtitle(subtitle);
    m_page->setBackVisible(false);

    auto* inner = new QWidget(m_page);
    auto* column = new QVBoxLayout(inner);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(0);
    column->addWidget(buildHeaderBar());
    column->addWidget(view, 1);
    view->setVisible(true);
    m_page->setBody(inner);

    addWidget(m_page);
    setCurrentWidget(m_page);

    m_pageTitle = title;
    m_pageSubtitle = subtitle;
    retranslate();
    return m_page;
}

StarPage* StarShell::currentPage() const
{
    return m_page;
}

StarNavRail::Page StarShell::currentRailPage() const
{
    return m_page ? m_page->railPage() : StarNavRail::Page::Start;
}

StarPage* StarShell::pushSubPage(QWidget* view, const QString& title, const QString& subtitle)
{
    if (m_subView) {
        popSubPage();
    }
    if (!m_page || !m_launcherPage) {
        return nullptr;
    }

    // park the launcher's view and show the transient one with a way back
    auto* inner = m_page->body();
    auto* column = qobject_cast<QVBoxLayout*>(inner->layout());
    column->removeWidget(m_launcherPage);
    m_launcherPage->setVisible(false);

    m_subView = view;
    column->addWidget(view, 1);
    m_subView->setVisible(true);

    m_page->setTitle(title);
    m_page->setSubtitle(subtitle);
    m_page->setBackVisible(true);
    m_page->setBackToolTip(tr("Back"));
    connect(m_page, &StarPage::backRequested, this, [this] { popSubPage(); });
    return m_page;
}

void StarShell::popSubPage()
{
    if (!m_subView || !m_page || !m_launcherPage) {
        m_subView = nullptr;
        return;
    }

    auto* inner = m_page->body();
    auto* column = qobject_cast<QVBoxLayout*>(inner->layout());
    if (column) {
        column->removeWidget(m_subView);
    }
    m_subView->deleteLater();
    m_subView = nullptr;

    column->addWidget(m_launcherPage, 1);
    m_launcherPage->setVisible(true);

    m_page->setBackVisible(false);
    m_page->setTitle(m_pageTitle);
    m_page->setSubtitle(m_pageSubtitle);
    emit subPageBack();
}

bool StarShell::hasSubPage() const
{
    return m_subView != nullptr;
}

QString StarShell::searchText() const
{
    return m_search ? m_search->text() : QString();
}

void StarShell::setSearchText(const QString& text)
{
    if (m_search) {
        m_search->setText(text);
    }
}

void StarShell::setAccountCaption(const QString& name)
{
    m_accountName = name;
    if (m_account) {
        m_account->setText(name.isEmpty() ? tr("Account") : name);
        m_account->setToolTip(name.isEmpty() ? tr("Accounts") : tr("Signed in as %1").arg(name));
    }
}

void StarShell::setPageSubtitle(const QString& subtitle)
{
    m_pageSubtitle = subtitle;
    if (m_page && !m_subView) {
        m_page->setSubtitle(subtitle);
    }
}

void StarShell::retranslate()
{
    if (m_search) {
        m_search->setPlaceholderText(tr("Search instances, worlds, mods"));
    }
    if (m_quickStart) {
        m_quickStart->setText(tr("Quick start"));
        m_quickStart->setToolTip(tr("Launch the last played instance"));
    }
    // re-presses the stored caption, so a language change re-translates it
    setAccountCaption(m_accountName);
}

void StarShell::retintBrand()
{
    if (!m_brandMark) {
        return;
    }
    // The SVG ships in the accent purple; the title strip wants the theme's
    // star colour, exactly like the rail's mark.
    const qreal dpr = qApp->devicePixelRatio();
    const QIcon source(QStringLiteral(":/starclient/icons/star-glyph.svg"));
    const QPixmap masked = source.pixmap(QSize(20, 20), dpr);
    if (masked.isNull()) {
        m_brandMark->clear();
        return;
    }
    const QColor window = QApplication::palette().color(QPalette::Window);
    const QColor color = window.lightness() > 128 ? QColor(0x0F, 0x13, 0x1C) : QColor(Qt::white);

    QPixmap out(masked.size());
    out.setDevicePixelRatio(dpr);
    out.fill(Qt::transparent);
    QPainter painter(&out);
    painter.drawPixmap(0, 0, masked);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(out.rect(), color);
    painter.end();
    m_brandMark->setPixmap(out);
}

void StarShell::changeEvent(QEvent* event)
{
    QStackedWidget::changeEvent(event);
    switch (event->type()) {
        case QEvent::LanguageChange:
            retranslate();
            break;
        case QEvent::PaletteChange:
        case QEvent::ApplicationPaletteChange:
        case QEvent::StyleChange:
            retintBrand();
            break;
        default:
            break;
    }
}
