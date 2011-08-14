#ifndef LUACONSTANTS_H
#define LUACONSTANTS_H

namespace LuaSupport {
namespace Constants {

// contexts
const char *const C_LUAEDITOR      = "Lua Editor";

// menus
const char * const M_CONTEXT = "LuaEditor.ContextMenu";

// actions
const char * const SEPARATOR           = "LuaEditor.Separator";
const char * const JUMP_TO_FILE        = "LuaEditor.JumpToFile";

const char *const LUA_EDITOR_ID    = "Premake.LuaEditor";
const char *const LUA_EDITOR_DISPLAY_NAME = QT_TRANSLATE_NOOP("OpenWith::Editors", "Lua Editor");

const char *const LUA_MIMETYPE     = "text/x-lua";

// ICONS
const char * const ICON_LUA_FILE   = ":/premakeproject/images/lua_small.png";

}
}

#endif // LUACONSTANTS_H
