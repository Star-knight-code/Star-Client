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

#include <QWidget>

#include "ui/StarNavRail.h"

class QLabel;
class QToolButton;

/**
 * A destination whose content is not built yet. Shows the page's name, its
 * accent star and one line about what lands there, so the rail is complete and
 * no destination is a blank space while the real views are still being framed.
 */
class StarPlaceholderView : public QWidget {
    Q_OBJECT

   public:
    explicit StarPlaceholderView(StarNavRail::Page page, QWidget* parent = nullptr);

    StarNavRail::Page page() const { return m_page; }
    virtual QString heading() const;
    virtual QString blurb() const;

    /// An action at the foot of the placeholder, when the destination deserves one.
    QToolButton* callToAction() const { return m_cta; }

   protected:
    void changeEvent(QEvent* event) override;

   private:
    void retint();

    StarNavRail::Page m_page;
    QLabel* m_star = nullptr;
    QLabel* m_heading = nullptr;
    QLabel* m_blurb = nullptr;
    QToolButton* m_cta = nullptr;
};
