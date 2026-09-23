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

class QLabel;

/**
 * The face of Star Start: a welcome, the main ways forward as action
 * cards, and a small summary of what is installed. Owns no state; every
 * button is a signal the main window routes.
 */
class StarHomeView : public QWidget {
    Q_OBJECT

   public:
    explicit StarHomeView(QWidget* parent = nullptr);

    /// One quiet line under the buttons: how many instances exist.
    void setSummary(const QString& summary);

   signals:
    void newInstanceRequested();
    void browseRequested();
    void accountsRequested();
    void settingsRequested();

   private:
    QLabel* m_summary = nullptr;
};
