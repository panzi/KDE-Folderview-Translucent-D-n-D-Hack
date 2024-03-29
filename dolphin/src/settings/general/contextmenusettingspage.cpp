/***************************************************************************
 *   Copyright (C) 2009 by Peter Penz <peter.penz19@gmail.com>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "contextmenusettingspage.h"

#include <dolphin_generalsettings.h>

#include <KDialog>
#include <KLocale>
#include <KVBox>

#include <QCheckBox>
#include <QVBoxLayout>

const bool SHOW_DELETE = false;

ContextMenuSettingsPage::ContextMenuSettingsPage(QWidget* parent) :
    SettingsPageBase(parent),
    m_showDeleteCommand(0),
    m_showCopyMoveMenu(0)
{
    QVBoxLayout* topLayout = new QVBoxLayout(this);
    KVBox* vBox = new KVBox(this);
    vBox->setSpacing(KDialog::spacingHint());

    m_showDeleteCommand = new QCheckBox(i18nc("@option:check", "Show 'Delete' command"), vBox);

    m_showCopyMoveMenu = new QCheckBox(i18nc("@option:check", "Show 'Copy To' and 'Move To' commands"), vBox);

    // Add a dummy widget with no restriction regarding
    // a vertical resizing. This assures that the dialog layout
    // is not stretched vertically.
    new QWidget(vBox);

    topLayout->addWidget(vBox);

    loadSettings();

    connect(m_showDeleteCommand, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
    connect(m_showCopyMoveMenu, SIGNAL(toggled(bool)), this, SIGNAL(changed()));
}

ContextMenuSettingsPage::~ContextMenuSettingsPage()
{
}

void ContextMenuSettingsPage::applySettings()
{
    KSharedConfig::Ptr globalConfig = KSharedConfig::openConfig("kdeglobals", KConfig::NoGlobals);
    KConfigGroup configGroup(globalConfig, "KDE");
    configGroup.writeEntry("ShowDeleteCommand", m_showDeleteCommand->isChecked());
    configGroup.sync();

    GeneralSettings::setShowCopyMoveMenu(m_showCopyMoveMenu->isChecked());
    GeneralSettings::self()->writeConfig();
}

void ContextMenuSettingsPage::restoreDefaults()
{
    GeneralSettings* settings = GeneralSettings::self();
    settings->useDefaults(true);
    loadSettings();
    settings->useDefaults(false);
    m_showDeleteCommand->setChecked(SHOW_DELETE);
}

void ContextMenuSettingsPage::loadSettings()
{
    KSharedConfig::Ptr globalConfig = KSharedConfig::openConfig("kdeglobals", KConfig::IncludeGlobals);
    KConfigGroup configGroup(globalConfig, "KDE");
    m_showDeleteCommand->setChecked(configGroup.readEntry("ShowDeleteCommand", SHOW_DELETE));

    m_showCopyMoveMenu->setChecked(GeneralSettings::showCopyMoveMenu());
}

#include "contextmenusettingspage.moc"
