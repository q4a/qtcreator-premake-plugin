--
-- This file is part of Qt Creator
--
-- Copyright (c) 2011 Konstantin Tokarev <annulen@yandex.ru>
--
-- Partially based on code of Qt Creator by Nokia Corporation
--
--
-- GNU Lesser General Public License Usage
--
-- This file may be used under the terms of the GNU Lesser General Public
-- License version 2.1 as published by the Free Software Foundation and
-- appearing in the file LICENSE.LGPL included in the packaging of this file.
-- Please review the following information to ensure the GNU Lesser General
-- Public License version 2.1 requirements will be met:
-- http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
--
-- In addition, as a special exception, Nokia gives you certain additional
-- rights. These rights are described in the Nokia Qt LGPL Exception
-- version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
--

_qtcreator_files = {}
_qtcreator_generated_files = {}
_qtcreator_includes = {}
_qtcreator_defines = {}
_qtcreator_scriptdepends = {}

function _qtcreator_projectname()
    return solution().name
end

function _qtcreator_rootpath()
    local maxUpDirCount = 0
    local deepPath = ''

    for sln in premake.solution.each()  do
        for _, prj in ipairs(sln.projects) do
            for _,file in ipairs(premake.getconfig(prj).files) do
                file = path.rebase(file, prj.location, prj.basedir)
                if not path.isabsolute(file) then
                    local upDirCount = 0
                    local cur = 0
                    repeat
                        local next = file:find("../", cur + 1, true)
                        if next then
                            cur = next
                            upDirCount = upDirCount + 1
                            if upDirCount >= maxUpDirCount then
                                maxUpDirCount = upDirCount
                                deepPath = prj.basedir
                            end
                        end
                    until (not next)
                end
            end
        end
    end

    if maxUpDirCount > 0 then
        for i = 1,maxUpDirCount do
            deepPath = path.join(deepPath, '..')
        end
        deepPath = path.getabsolute(deepPath)
        return deepPath
    else
        return ""
    end
end

newaction {
    trigger = '_qtcreator',
    isinternal = true,
    depends = { 'qt' },
    onproject = function(prj)
        print("Starting project " .. prj.name)
        -- TODO: Query Makefile variable values directly

        -- Uses first configuration
        -- FIXME: use active configuration
        local cfgname = prj.solution.configurations[1]
        local cfg = premake.getconfig(prj, cfgname, "Native")

        local mocdir = path.join(prj.location, cfg.mocdir or cfg.objectsdir)
        local uidir = path.join(prj.location, cfg.uidir or cfg.objectsdir)
        local rccdir = path.join(prj.location, cfg.rccdir or cfg.objectsdir)
        local qmdir = path.join(prj.location, cfg.qmdir or cfg.objectsdir)

        prj.qt_generated_files_keys = prj.qt_generated_files_keys or {}
        for _,file in ipairs(prj.files) do
            local f = path.getabsolute(path.join(prj.location, file))
            if prj.qt_generated_files_keys[f] then
                f = f:gsub("%$%(MOCDIR%)", mocdir)
                f = f:gsub("%$%(UIDIR%)", uidir)
                f = f:gsub("%$%(RCCDIR%)", rccdir)
                f = f:gsub("%$%(QMDIR%)", qmdir)
                table.insert(_qtcreator_generated_files, f)
            end
            table.insert(_qtcreator_files, f)
        end
        for _,def in ipairs(cfg.defines) do
            table.insert(_qtcreator_defines, def)
        end
        for _,idir in ipairs(cfg.includedirs) do
            idir = idir:gsub("%$%(MOCDIR%)", mocdir)
            idir = idir:gsub("%$%(UIDIR%)", uidir)
            idir = path.getabsolute(path.join(prj.location, idir))
            table.insert(_qtcreator_includes, idir)
        end
    end
}

