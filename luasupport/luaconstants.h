#ifndef LUACONSTANTS_H
#define LUACONSTANTS_H

#include <QtGlobal>

#ifdef USE_APP_VERSION
#include <app/app_version.h>
#else
#include <ide_version.h>
#endif

#define IDE_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
#define IDE_VER IDE_VERSION_CHECK(IDE_VERSION_MAJOR, IDE_VERSION_MINOR, IDE_VERSION_RELEASE)

#if IDE_VER >= IDE_VERSION_CHECK(2, 4, 80)
#include <coreplugin/idocument.h>
#else
#include <coreplugin/ifile.h>
namespace Core {
    typedef IFile IDocument;
}
#endif

namespace LuaSupport {
namespace Constants {

// contexts
const char C_LUAEDITOR[]      = "Lua Editor";

// menus
const char M_CONTEXT[]        = "LuaEditor.ContextMenu";

// actions
const char SEPARATOR[]        = "LuaEditor.Separator";
const char JUMP_TO_FILE[]     = "LuaEditor.JumpToFile";

const char LUA_EDITOR_ID[]    = "Premake.LuaEditor";
const char LUA_EDITOR_DISPLAY_NAME[] = QT_TRANSLATE_NOOP("OpenWith::Editors", "Lua Editor");

const char LUA_MIMETYPE[]     = "text/x-lua";

// ICONS
const char ICON_LUA_FILE[]    = ":/premakeproject/images/lua_small.png";

}
}

#endif // LUACONSTANTS_H
