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
#include "StarIcons.h"

#include <QApplication>
#include <QColor>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QSize>

namespace StarIcons {

QIcon pageIcon(const QString& name)
{
    const int size = 20;
    const qreal dpr = qApp ? qApp->devicePixelRatio() : 1.0;
    const QIcon source(QStringLiteral(":/starclient/icons/sidebar-%1.svg").arg(name));
    const QPixmap masked = source.pixmap(QSize(size, size), dpr);
    if (masked.isNull()) {
        return source;
    }

    QPixmap out(masked.size());
    out.setDevicePixelRatio(dpr);
    out.fill(Qt::transparent);
    QPainter painter(&out);
    painter.drawPixmap(0, 0, masked);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(out.rect(), QApplication::palette().color(QPalette::WindowText));
    painter.end();

    QIcon icon;
    icon.addPixmap(out);
    return icon;
}

}  // namespace StarIcons
