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
#include "StarHomeView.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>
#include <QRectF>
#include <QSizePolicy>
#include <QToolButton>
#include <QVBoxLayout>

namespace {

/// The mark in the theme's star colour, at hero size.
QPixmap heroStar()
{
    const int size = 44;
    const qreal dpr = qApp->devicePixelRatio();
    QPixmap pixmap(QSize(size, size) * dpr);
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

    const QColor window = QApplication::palette().color(QPalette::Window);
    const QColor color = window.lightness() > 128 ? QColor(0x0F, 0x13, 0x1C) : QColor(Qt::white);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.scale(size / 256.0, size / 256.0);
    QPainterPath star;
    star.moveTo(128.0, 16.0);
    star.quadTo(150.4, 105.6, 240.0, 128.0);
    star.quadTo(150.4, 150.4, 128.0, 240.0);
    star.quadTo(105.6, 150.4, 16.0, 128.0);
    star.quadTo(105.6, 105.6, 128.0, 16.0);
    star.closeSubpath();
    painter.drawPath(star);
    painter.end();
    return pixmap;
}

}  // namespace

StarHomeView::StarHomeView(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* column = new QVBoxLayout(this);
    column->setContentsMargins(48, 48, 48, 48);
    column->setSpacing(0);

    column->addStretch(1);

    auto* mark = new QLabel(this);
    mark->setPixmap(heroStar());
    mark->setAlignment(Qt::AlignCenter);
    column->addWidget(mark, 0, Qt::AlignHCenter);

    column->addSpacing(20);

    auto* heading = new QLabel(tr("Welcome to Star Client"), this);
    heading->setProperty("starRole", QStringLiteral("h1"));
    heading->setAlignment(Qt::AlignCenter);
    column->addWidget(heading, 0, Qt::AlignHCenter);

    column->addSpacing(10);

    auto* blurb = new QLabel(tr("Everything starts here: make an instance, install a pack or sign in."),
                             this);
    blurb->setProperty("starRole", QStringLiteral("muted"));
    blurb->setAlignment(Qt::AlignCenter);
    blurb->setWordWrap(true);
    column->addWidget(blurb, 0, Qt::AlignHCenter);

    column->addSpacing(34);

    auto* row = new QHBoxLayout();
    row->setSpacing(12);

    auto makeButton = [this](const QString& text, const char* role) -> QToolButton* {
        auto* button = new QToolButton(this);
        button->setProperty("starRole", QString::fromLatin1(role));
        button->setCursor(Qt::PointingHandCursor);
        button->setText(text);
        button->setToolTip(text);
        return button;
    };

    auto* newInstance = makeButton(tr("New instance"), "primary");
    auto* browse = makeButton(tr("Browse packs"), "outline");
    auto* accounts = makeButton(tr("Add account"), "outline");
    auto* settings = makeButton(tr("Launcher settings"), "ghost");

    row->addStretch(1);
    row->addWidget(newInstance);
    row->addWidget(browse);
    row->addWidget(accounts);
    row->addWidget(settings);
    row->addStretch(1);
    column->addLayout(row);

    connect(newInstance, &QToolButton::clicked, this, &StarHomeView::newInstanceRequested);
    connect(browse, &QToolButton::clicked, this, &StarHomeView::browseRequested);
    connect(accounts, &QToolButton::clicked, this, &StarHomeView::accountsRequested);
    connect(settings, &QToolButton::clicked, this, &StarHomeView::settingsRequested);

    column->addSpacing(26);

    m_summary = new QLabel(this);
    m_summary->setProperty("starRole", QStringLiteral("muted"));
    m_summary->setAlignment(Qt::AlignCenter);
    column->addWidget(m_summary, 0, Qt::AlignHCenter);

    column->addStretch(2);
}

void StarHomeView::setSummary(const QString& summary)
{
    if (m_summary) {
        m_summary->setText(summary);
    }
}
