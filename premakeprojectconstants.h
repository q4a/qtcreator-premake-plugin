/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (info@qt.nokia.com)
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**************************************************************************/

#ifndef PREMAKEPROJECTCONSTANTS_H
#define PREMAKEPROJECTCONSTANTS_H

#include <QtCore/QtGlobal>
#ifdef USE_APP_VERSION
#include <app/app_version.h>
#else
#include <ide_version.h>
#endif

#define IDE_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
#define IDE_VER IDE_VERSION_CHECK(IDE_VERSION_MAJOR, IDE_VERSION_MINOR, IDE_VERSION_RELEASE)

namespace PremakeProjectManager {
namespace Constants {

const char *const PROJECTCONTEXT     = "PremakeProject.ProjectContext";
const char *const PREMAKEMIMETYPE    = "application/vnd.premake";

// kinds
const char *const PROJECT_KIND       = "Premake";

// Project
const char *const PREMAKEPROJECT_ID  = "PremakeProjectManager.PremakeProject";

// ICONS
const char * const ICON_PREMAKE   = ":/premakeproject/images/premake.png";
const char * const ICON_PREMAKEPROJECT   = ":/premakeproject/images/premakeproject.png";

} // namespace Constants
} // namespace PremakeProjectManager

#endif // PREMAKEPROJECTCONSTANTS_H
