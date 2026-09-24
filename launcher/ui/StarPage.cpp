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
#include "StarPage.h"

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPalette>
#include <QToolButton>
#include <QVBoxLayout>


StarPage::StarPage(StarNavRail::Page page, QWidget* parent) : QWidget(parent), m_page(page)
{
    // the surface step of the elevation ladder; the stylesheet styles it
    setProperty("starRole", QStringLiteral("surface"));

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(28, 24, 28, 24);
    outer->setSpacing(0);

    auto* header = new QHBoxLayout();
    header->setSpacing(14);

    m_backButton = new QToolButton(this);
    m_backButton->setObjectName(QStringLiteral("starBack"));
    m_backButton->setProperty("starRole", QStringLiteral("ghost"));
    m_backButton->setAutoRaise(true);
    m_backButton->setCursor(Qt::PointingHandCursor);
    m_backButton->setIcon(QIcon(QStringLiteral(":/starclient/icons/back.svg")));
    m_backButton->setText(QString());
    m_backButton->setVisible(false);
    connect(m_backButton, &QToolButton::clicked, this, &StarPage::backRequested);
    header->addWidget(m_backButton, 0, Qt::AlignLeft | Qt::AlignVCenter);

    auto* titling = new QVBoxLayout();
    titling->setSpacing(2);
    m_title = new QLabel(this);
    m_title->setProperty("starRole", QStringLiteral("h1"));
    titling->addWidget(m_title);

    m_subtitle = new QLabel(this);
    m_subtitle->setProperty("starRole", QStringLiteral("subtitle"));
    m_subtitle->setWordWrap(true);
    titling->addWidget(m_subtitle);

    header->addLayout(titling, 1);

    m_headerActions = new QHBoxLayout();
    m_headerActions->setSpacing(10);
    m_headerActions->setContentsMargins(0, 0, 0, 0);
    auto* actionsHolder = new QWidget(this);
    actionsHolder->setLayout(m_headerActions);
    header->addWidget(actionsHolder, 0, Qt::AlignRight | Qt::AlignVCenter);

    outer->addLayout(header);

    setTitle(displayTitle(page));
}

QString StarPage::displayTitle(StarNavRail::Page page) const
{
    switch (page) {
        case StarNavRail::Page::Start:
            return tr("Star Start");
        case StarNavRail::Page::Instances:
            return tr("Instances");
        case StarNavRail::Page::Browse:
            return tr("Browse");
        case StarNavRail::Page::Accounts:
            return tr("Accounts");
        case StarNavRail::Page::Edit:
            return tr("Edit");
        case StarNavRail::Page::Settings:
            return tr("Settings");
    }
    return {};
}

void StarPage::setTitle(const QString& title)
{
    m_title->setText(title);
}

QString StarPage::title() const
{
    return m_title->text();
}

void StarPage::setSubtitle(const QString& subtitle)
{
    m_subtitle->setText(subtitle);
    m_subtitle->setVisible(!subtitle.isEmpty());
}

QToolButton* StarPage::addHeaderAction(const QString& iconName, const QString& text, const QString& role)
{
    auto* button = new QToolButton(this);
    button->setProperty("starRole", role);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setCursor(Qt::PointingHandCursor);
    button->setText(text);
    button->setToolTip(text);
    if (!iconName.isEmpty()) {
        button->setIcon(QIcon(QStringLiteral(":/starclient/icons/%1.svg").arg(iconName)));
    }
    m_headerActions->addWidget(button);
    return button;
}

void StarPage::setBody(QWidget* body)
{
    body->setParent(this);
    m_body = body;

    // reserved a slot at the bottom of the page for the content
    auto* outer = qobject_cast<QVBoxLayout*>(layout());
    outer->addSpacing(18);
    outer->addWidget(body, 1);
}

void StarPage::setBackVisible(bool visible)
{
    m_backButton->setVisible(visible);
}

bool StarPage::isBackVisible() const
{
    return m_backButton->isVisible();
}

void StarPage::setBackEnabled(bool enabled)
{
    m_backButton->setEnabled(enabled);
}

void StarPage::setBackToolTip(const QString& text)
{
    m_backButton->setToolTip(text);
}

void StarPage::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    // reserved for theme-sensitive painting in a later step; everything the
    // page shows today is driven by the stylesheet
}
