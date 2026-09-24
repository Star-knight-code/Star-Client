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
#include "StarNavRail.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QEvent>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPixmap>
#include <QRectF>
#include <QSizePolicy>
#include <QToolButton>
#include <QTransform>
#include <QWidget>

#include "rainbow.h"

namespace {

/// Glyph size in the rail, and the size the mark is drawn at.
constexpr int kGlyphSize = 22;
constexpr int kMarkSize = 26;

/// How far an idle or hovered glyph is blended towards the window colour.
constexpr qreal kIdleFade = 0.45;
constexpr qreal kHoverFade = 0.15;

/// The theme's star colour: white on the dark themes, ink on the light ones,
/// the same way the prototype drives it from currentColor.
QColor starColor()
{
    const QColor window = QApplication::palette().color(QPalette::Window);
    return window.lightness() > 128 ? QColor(0x0F, 0x13, 0x1C) : QColor(Qt::white);
}

QColor glyphColor(qreal fade)
{
    const QPalette palette = QApplication::palette();
    return Rainbow::mix(palette.color(QPalette::WindowText), palette.color(QPalette::Window), fade);
}

/// Replaces every painted pixel of an icon with a single colour. The design
/// system draws its glyphs with one stroke colour, so this is lossless.
QPixmap tinted(const QIcon& source, const QColor& color, int size, qreal dpr)
{
    const QPixmap masked = source.pixmap(QSize(size, size), dpr);
    if (masked.isNull()) {
        return {};
    }

    QPixmap out(masked.size());
    out.setDevicePixelRatio(dpr);
    out.fill(Qt::transparent);

    QPainter painter(&out);
    painter.drawPixmap(0, 0, masked);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(out.rect(), color);
    painter.end();
    return out;
}

/// The four-pointed star, using the prototype's canonical geometry
/// (a 256 unit box, so the mark keeps its proportions at any size).
QPainterPath starPath(const QRectF& box)
{
    QPainterPath path;
    path.moveTo(128.0, 16.0);
    path.quadTo(150.4, 105.6, 240.0, 128.0);
    path.quadTo(150.4, 150.4, 128.0, 240.0);
    path.quadTo(105.6, 150.4, 16.0, 128.0);
    path.quadTo(105.6, 105.6, 128.0, 16.0);
    path.closeSubpath();

    QTransform transform;
    transform.translate(box.x(), box.y());
    transform.scale(box.width() / 256.0, box.height() / 256.0);
    return transform.map(path);
}

}  // namespace

QString StarNavRail::pageTitle(Page page)
{
    switch (page) {
        case Page::Start:
            return tr("Star Start");
        case Page::Instances:
            return tr("Instances");
        case Page::Browse:
            return tr("Browse content");
        case Page::Accounts:
            return tr("Accounts");
        case Page::Edit:
            return tr("Edit");
        case Page::Settings:
            return tr("Settings");
    }
    return {};
}

StarNavRail::StarNavRail(QWidget* parent) : QToolBar(tr("Navigation"), parent)
{
    // The QSS picks the rail out by this name.
    setObjectName(QStringLiteral("starNavRail"));
    setOrientation(Qt::Vertical);
    setMovable(false);
    setFloatable(false);
    setIconSize(QSize(kGlyphSize, kGlyphSize));
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    buildActions();
    setCurrentPage(Page::Start);
    retint();
}

QAction* StarNavRail::addPageAction(Page page, const QString& iconName)
{
    const QString text = pageTitle(page);

    auto* action = new QAction(text, this);
    action->setCheckable(true);
    action->setToolTip(text);
    action->setData(static_cast<int>(page));
    addAction(action);
    m_pageActions.append(action);
    m_pages->addAction(action);

    // the glyph is filled in by retint()
    action->setProperty("starIconName", iconName);
    if (auto* button = qobject_cast<QToolButton*>(widgetForAction(action))) {
        button->setAccessibleName(text);
    }
    return action;
}

void StarNavRail::buildActions()
{
    // The mark is not a destination on its own: it goes back to Star Start,
    // the same way clicking the logo does in the prototype.
    m_starAction = new QAction(tr("Star Client"), this);
    m_starAction->setToolTip(tr("Star Start"));
    addAction(m_starAction);
    connect(m_starAction, &QAction::triggered, this, &StarNavRail::startRequested);

    m_pages = new QActionGroup(this);
    m_pages->setExclusive(true);

    QAction* start = addPageAction(Page::Start, QStringLiteral("rail-home"));
    QAction* instances = addPageAction(Page::Instances, QStringLiteral("rail-grid"));
    QAction* browse = addPageAction(Page::Browse, QStringLiteral("rail-compass"));
    QAction* accounts = addPageAction(Page::Accounts, QStringLiteral("rail-users"));

    connect(start, &QAction::triggered, this, &StarNavRail::startRequested);
    connect(instances, &QAction::triggered, this, &StarNavRail::instancesRequested);
    connect(browse, &QAction::triggered, this, &StarNavRail::browseRequested);
    connect(accounts, &QAction::triggered, this, &StarNavRail::accountsRequested);

    addSeparator();

    // Everything below is pushed to the bottom of the rail.
    auto* spacer = new QWidget(this);
    spacer->setObjectName(QStringLiteral("starNavSpacer"));
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    addWidget(spacer);

    addBottomActions();
}

void StarNavRail::addBottomActions()
{
    m_appearanceAction = new QAction(tr("Switch appearance"), this);
    m_appearanceAction->setToolTip(tr("Switch appearance"));
    addAction(m_appearanceAction);
    connect(m_appearanceAction, &QAction::triggered, this, &StarNavRail::appearanceToggleRequested);

    m_settingsAction = addPageAction(Page::Settings, QStringLiteral("rail-cog"));
    connect(m_settingsAction, &QAction::triggered, this, &StarNavRail::settingsRequested);
}

void StarNavRail::dressButton(QToolButton* button)
{
    if (!button) {
        return;
    }
    button->setAutoRaise(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setCursor(Qt::PointingHandCursor);
}

void StarNavRail::setCurrentPage(Page page)
{
    m_currentPage = page;
    for (QAction* action : m_pageActions) {
        action->setChecked(action->data().toInt() == static_cast<int>(page));
    }
}

QIcon StarNavRail::railIcon(const QString& resourcePath, int size)
{
    const qreal dpr = qApp->devicePixelRatio();
    const QIcon source(resourcePath);

    QIcon icon;
    // Idle, hovered and selected states, so a stylesheet does not have to
    // recolour anything.
    icon.addPixmap(tinted(source, glyphColor(kIdleFade), size, dpr), QIcon::Normal, QIcon::Off);
    icon.addPixmap(tinted(source, glyphColor(kHoverFade), size, dpr), QIcon::Active, QIcon::Off);
    icon.addPixmap(tinted(source, QApplication::palette().color(QPalette::Highlight), size, dpr), QIcon::Normal, QIcon::On);
    icon.addPixmap(tinted(source, QApplication::palette().color(QPalette::Highlight), size, dpr), QIcon::Active, QIcon::On);
    icon.addPixmap(tinted(source, QApplication::palette().color(QPalette::Highlight), size, dpr), QIcon::Selected, QIcon::On);
    return icon;
}

QIcon StarNavRail::starMarkIcon(int size)
{
    const qreal dpr = qApp->devicePixelRatio();
    QPixmap pixmap(QSize(size, size) * dpr);
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(starColor());
    painter.drawPath(starPath(QRectF(0, 0, size, size)));
    painter.end();

    return QIcon(pixmap);
}

void StarNavRail::retint()
{
    if (!m_starAction) {
        return;
    }

    m_starAction->setIcon(starMarkIcon(kMarkSize));
    if (auto* button = qobject_cast<QToolButton*>(widgetForAction(m_starAction))) {
        dressButton(button);
    }

    const auto tintOne = [this](QAction* action, int size) {
        const QString name = action->property("starIconName").toString();
        if (name.isEmpty()) {
            return;
        }
        action->setIcon(railIcon(QStringLiteral(":/starclient/icons/%1.svg").arg(name), size));
        if (auto* button = qobject_cast<QToolButton*>(widgetForAction(action))) {
            dressButton(button);
        }
    };

    for (QAction* action : m_pageActions) {
        tintOne(action, kGlyphSize);
    }
    if (m_appearanceAction) {
        m_appearanceAction->setProperty("starIconName", QStringLiteral("rail-moon"));
        tintOne(m_appearanceAction, kGlyphSize);
    }
}

void StarNavRail::retranslate()
{
    m_starAction->setToolTip(tr("Star Start"));

    for (QAction* action : m_pageActions) {
        const QString text = pageTitle(static_cast<Page>(action->data().toInt()));
        action->setText(text);
        action->setToolTip(text);
        if (auto* button = qobject_cast<QToolButton*>(widgetForAction(action))) {
            button->setAccessibleName(text);
        }
    }

    if (m_appearanceAction) {
        m_appearanceAction->setToolTip(tr("Switch appearance"));
    }
}

void StarNavRail::changeEvent(QEvent* event)
{
    QToolBar::changeEvent(event);

    switch (event->type()) {
        case QEvent::PaletteChange:
        case QEvent::ApplicationPaletteChange:
        case QEvent::StyleChange:
        case QEvent::ScreenChangeInternal:
            retint();
            break;
        case QEvent::LanguageChange:
            retranslate();
            break;
        default:
            break;
    }
}
