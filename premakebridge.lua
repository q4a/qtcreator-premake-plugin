--
-- This file is part of Qt Creator
--
-- Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
--
-- Contact: Nokia Corporation (info@qt.nokia.com)
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
-- Other Usage
--
-- Alternatively, this file may be used in accordance with the terms and
-- conditions contained in a signed written agreement between you and Nokia.
--
-- If you have questions regarding the use of this file, please contact
-- Nokia at qt-info@nokia.com.
--

_qtcreator_files = {}
_qtcreator_includes = {}
_qtcreator_defines = {}

function _qtcreator_projectname()
    return solution().name
end

function _qtcreator_rootpath()
    local maxUpDirCount = 0
    local deepPath = ''

    for sln in premake.solution.each()  do
        for _, prj in ipairs(sln.projects) do
            for _,file in ipairs(premake.getconfig(prj).files) do
                if not path.isabsolute(file) then
                    local upDirCount = 0
                    for n,part in ipairs(file:explode('/', true)) do
                        if part ~= '..' then break end
                        if n > upDirCount then
                            upDirCount = n
                        end
                        if upDirCount >= maxUpDirCount then
                            maxUpDirCount = upDirCount
                            deepPath = prj.location
                        end
                    end
                    --print(file, upDirCount)
                end
                print("maxUpDirCount = ", maxUpDirCount)
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
    description = '',
    execute = function()
        for sln in premake.solution.each()  do
            for _, prj in ipairs(sln.projects) do
                for _,file in ipairs(premake.getconfig(prj).files) do
                    table.insert(_qtcreator_files, path.getabsolute(path.join(prj.location, file)))
                end
                for _,def in ipairs(premake.getconfig(prj).defines) do
                    table.insert(_qtcreator_defines, def)
                end
                for _,idir in ipairs(premake.getconfig(prj).includedirs) do
                    table.insert(_qtcreator_includes, idir)
                end
            end
        end
    end
}
