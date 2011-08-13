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
#include <ide_version.h>

#define IDE_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
#define IDE_VER IDE_VERSION_CHECK(IDE_VERSION_MAJOR, IDE_VERSION_MINOR, IDE_VERSION_RELEASE)

namespace PremakeProjectManager {
namespace Constants {

const char *const PROJECTCONTEXT     = "PremakeProject.ProjectContext";
const char *const PREMAKEMIMETYPE    = "text/x-premake";

// contexts
const char *const C_LUAEDITOR      = "Lua Editor";

// menus
const char * const M_CONTEXT = "LuaEditor.ContextMenu";

// actions
const char * const SEPARATOR           = "LuaEditor.Separator";
const char * const JUMP_TO_FILE        = "LuaEditor.JumpToFile";

// kinds
const char *const PROJECT_KIND       = "Premake";

const char *const LUA_EDITOR_ID    = "Premake.LuaEditor";
const char *const LUA_EDITOR_DISPLAY_NAME = QT_TRANSLATE_NOOP("OpenWith::Editors", "Lua Editor");

const char *const LUA_MIMETYPE     = "text/x-lua";

// Project
const char *const PREMAKEPROJECT_ID  = "PremakeProjectManager.PremakeProject";

// ICONS
const char * const ICON_PREMAKE   = ":/premakeproject/images/premake.png";
const char * const ICON_LUA_FILE   = ":/premakeproject/images/lua_small.png";

} // namespace Constants
} // namespace PremakeProjectManager

#endif // PREMAKEPROJECTCONSTANTS_H
