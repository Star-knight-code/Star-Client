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
#include "StarPlaceholderView.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>
#include <QRectF>
#include <QToolButton>
#include <QVBoxLayout>

namespace {

/// The star in the destination's accent, drawn exactly as the rail paints it.
QPixmap accentStar(const QColor& color, int size)
{
    QPixmap pixmap(QSize(size, size) * qApp->devicePixelRatio());
    pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);

    const qreal scale = size / 256.0;
    painter.scale(scale, scale);
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

StarPlaceholderView::StarPlaceholderView(StarNavRail::Page page, QWidget* parent) : QWidget(parent), m_page(page)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* column = new QVBoxLayout(this);
    column->setContentsMargins(32, 32, 32, 32);
    column->setSpacing(0);

    column->addStretch(1);

    m_star = new QLabel(this);
    m_star->setAlignment(Qt::AlignCenter);
    column->addWidget(m_star, 0, Qt::AlignHCenter);

    column->addSpacing(22);

    m_heading = new QLabel(heading(), this);
    m_heading->setProperty("starRole", QStringLiteral("h1"));
    m_heading->setAlignment(Qt::AlignCenter);
    m_heading->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    column->addWidget(m_heading, 0, Qt::AlignHCenter);

    column->addSpacing(10);

    m_blurb = new QLabel(blurb(), this);
    m_blurb->setProperty("starRole", QStringLiteral("muted"));
    m_blurb->setAlignment(Qt::AlignCenter);
    m_blurb->setWordWrap(true);
    m_blurb->setMaximumWidth(660);
    m_blurb->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    column->addWidget(m_blurb, 0, Qt::AlignHCenter);

    column->addSpacing(30);

    m_cta = new QToolButton(this);
    m_cta->setProperty("starRole", QStringLiteral("outline"));
    m_cta->setCursor(Qt::PointingHandCursor);
    m_cta->setVisible(false);
    column->addWidget(m_cta, 0, Qt::AlignHCenter);

    column->addStretch(1);

    retint();
}

QString StarPlaceholderView::heading() const
{
    switch (m_page) {
        case StarNavRail::Page::Browse:
            return tr("Browse content");
        case StarNavRail::Page::Accounts:
            return tr("Accounts");
        case StarNavRail::Page::Settings:
            return tr("Settings");
        default:
            break;
    }
    return {};
}

QString StarPlaceholderView::blurb() const
{
    switch (m_page) {
        case StarNavRail::Page::Browse:
            return tr("The catalogue of modpacks and resources lands here. Until it is framed, the installer "
                      "opens in its own window from above.");
        case StarNavRail::Page::Accounts:
            return tr("Your Microsoft and Mojang sign-ins land here. Until this page is framed, they are managed "
                      "from the launcher's settings.");
        case StarNavRail::Page::Settings:
            return tr("The launcher's settings land here, redesigned. Until that page is framed, they open in "
                      "their own window.");
        default:
            break;
    }
    return {};
}

void StarPlaceholderView::retint()
{
    // The mark keeps the launcher's star colour: white on the dark themes,
    // ink on the light ones; it is not a place for the accent.
    const QColor window = QApplication::palette().color(QPalette::Window);
    const QColor star = window.lightness() > 128 ? QColor(0x0F, 0x13, 0x1C) : QColor(Qt::white);
    m_star->setPixmap(accentStar(star, 72));
}

void StarPlaceholderView::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    switch (event->type()) {
        case QEvent::PaletteChange:
        case QEvent::ApplicationPaletteChange:
            retint();
            break;
        default:
            break;
    }
}
