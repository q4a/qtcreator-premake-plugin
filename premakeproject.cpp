/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Konstantin Tokarev <annulen@yandex.ru>
**
** Partially based on code of Qt Creator by Nokia Corporation
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

#include "premakeproject.h"

#include "premakebuildconfiguration.h"
#include "premakeprojectconstants.h"
#include "premaketarget.h"
#include "luamanager.h"

#include <projectexplorer/buildenvironmentwidget.h>
#include <projectexplorer/headerpath.h>
#include <projectexplorer/customexecutablerunconfiguration.h>
#include <projectexplorer/toolchainmanager.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <cplusplus/ModelManagerInterface.h>
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/messagemanager.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QProcessEnvironment>

using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;
using namespace ProjectExplorer;
using namespace LuaSupport;

namespace {
const char * const TOOLCHAIN_KEY("PremakeProjectManager.PremakeProject.Toolchain");
} // end of anonymous namespace

////////////////////////////////////////////////////////////////////////////////////
// PremakeProject
////////////////////////////////////////////////////////////////////////////////////

PremakeProject::PremakeProject(PremakeManager *manager, const QString &fileName)
    : m_manager(manager),
      m_fileName(fileName),
      m_toolChain(0)
{
    m_file = new PremakeProjectFile(this, fileName);
    m_rootNode = new PremakeProjectNode(this);
    m_manager->registerProject(this);
}

PremakeProject::~PremakeProject()
{
    m_codeModelFuture.cancel();
    m_manager->unregisterProject(this);

    delete m_rootNode;
    // do not delete m_toolChain
}

PremakeTarget *PremakeProject::activeTarget() const
{
    return static_cast<PremakeTarget *>(Project::activeTarget());
}

bool PremakeProject::addFiles(const QStringList &filePaths)
{
    return false;
}

bool PremakeProject::removeFiles(const QStringList &filePaths)
{
    return false;
}

static void getLuaTable(lua_State *L, const char *tablename, QStringList &to)
{
    lua_getglobal(L, tablename);
    // get "table" table
    lua_pushstring(L, "table");
    lua_gettable(L, LUA_GLOBALSINDEX);
    // get "getn" entry
    lua_pushstring(L, "getn");
    lua_gettable(L, -2);
    // call it and get result
    lua_pushvalue(L, -3); // -3 is our table
    if(lua_pcall(L, 1, 1, 0) != 0)
        qWarning() << lua_tostring(L, -1);
    int n_elements = lua_tonumber(L, -1);
    lua_pop(L, 2);
    to.clear();
    for(int i=0; i<n_elements; ++i) {
        lua_pushinteger(L, i+1); // Lua index starts from 1
        lua_gettable(L, -2);
        QString filename = QString::fromLocal8Bit(lua_tolstring(L, -1, 0));
        lua_pop(L, 1);
        to << filename; //m_rootDir.absoluteFilePath(filename);
    }
}

static void projectParseError(const QString &errorMessage)
{
    Core::ICore::instance()->messageManager()->printToOutputPanePopup(
            QCoreApplication::translate("PremakeProject", "Premake error: ") + errorMessage);
}
static void projectParseError(const char *errorMessage)
{
    projectParseError(QString::fromLocal8Bit(errorMessage));
}

void PremakeProject::parseProject(RefreshOptions options)
{
    const PremakeBuildConfiguration *conf = activeTarget()->activeBuildConfiguration();

    const QHash<QString,QString> qtInfo
            = conf->qtVersion()->versionInfo();

    /// @todo Create a persistent lua state with all built-in scripts loaded
    /// and clone it before loading project
    lua_State *L = LuaManager::instance()->initLuaState(m_fileName, "_qtcreator",
                                                        conf->shadowBuildEnabled(),
                                                        conf->buildDirectory());

    if(call_premake_main(L) != 0){
        projectParseError(lua_tostring(L, -1));
    }

    if (options & ProjectName) {
        lua_getfield(L, LUA_GLOBALSINDEX, "_qtcreator_projectname");
        if(lua_pcall(L, 0, 1, 0) != 0) {
            projectParseError(lua_tostring(L, -1));
        } else {
            m_projectName = QString::fromLocal8Bit(lua_tostring(L, -1));
            m_rootNode->setDisplayName(m_projectName);
        }
        lua_pop(L, 1);
    }

    if(options & RootPath) {
        lua_getfield(L, LUA_GLOBALSINDEX, "_qtcreator_rootpath");
        if(lua_pcall(L, 0, 1, 0) != 0)
            projectParseError(lua_tostring(L, -1));
        else
            m_rootNode->setPath(QString::fromLocal8Bit(lua_tostring(L, -1)));
        lua_pop(L, 1);
    }

    if (options & Files) {
        getLuaTable(L, "_qtcreator_files", m_files);
        getLuaTable(L, "_qtcreator_generated_files", m_generated);
        getLuaTable(L, "_qtcreator_scriptdepends", m_scriptDepends);
        m_scriptDepends.removeDuplicates();
        foreach(const QString &file, m_files) {
            m_scriptDepends.removeAll(file);
        }
    }

    if (options & Configuration) {
        getLuaTable(L, "_qtcreator_includes", m_includePaths);
        m_includePaths.removeDuplicates();
        for (int i=0; i<m_includePaths.size(); ++i) {
            m_includePaths[i].replace(QLatin1String("$(QT_INCLUDE)"), qtInfo.value(QLatin1String("QT_INSTALL_HEADERS")));
            m_includePaths[i].replace(QLatin1String("$(QT_LIB)"), qtInfo.value(QLatin1String("QT_INSTALL_LIBS")));
        }
//        qDebug() << Q_FUNC_INFO << "m_includePaths=" << m_includePaths;

        QStringList defines;
        getLuaTable(L, "_qtcreator_defines", defines);
        defines.removeDuplicates();
        m_defines.clear();
        foreach(const QString &def, defines)
            m_defines += QString::fromLatin1("#define %1\n").arg(def).replace(QLatin1Char('='), QLatin1Char(' ')).toLocal8Bit();
//        qWarning() << m_defines;
    }

    if (options & Files)
        emit fileListChanged();

    lua_close(L);
}

void PremakeProject::refresh(RefreshOptions options)
{
    QSet<QString> oldFileList;
    if (!(options & Configuration))
        oldFileList = m_files.toSet();

    parseProject(options);

    if (options & Files)
        m_rootNode->refresh();

    CPlusPlus::CppModelManagerInterface *modelManager =
        CPlusPlus::CppModelManagerInterface::instance();

    if (modelManager) {
        CPlusPlus::CppModelManagerInterface::ProjectInfo pinfo = modelManager->projectInfo(this);

        ToolChain *tc = activeTarget()->activeBuildConfiguration()->toolChain();
        if (tc) {
            pinfo.defines = tc->predefinedMacros();
            pinfo.defines += '\n';

            foreach (const HeaderPath &headerPath, tc->systemHeaderPaths()) {
                if (headerPath.kind() == HeaderPath::FrameworkHeaderPath)
                    pinfo.frameworkPaths.append(headerPath.path());
                else
                    pinfo.includePaths.append(headerPath.path());
            }
        }

        pinfo.includePaths += allIncludePaths();
        pinfo.defines += m_defines;

        // ### add _defines.
        pinfo.sourceFiles = files();
        //pinfo.sourceFiles += generated();

        QStringList filesToUpdate;

        if (options & Configuration) {
            filesToUpdate = pinfo.sourceFiles;
            filesToUpdate.prepend(QLatin1String("<configuration>")); // XXX don't hardcode configuration file name
            // Full update, if there's a code model update, cancel it
            m_codeModelFuture.cancel();
        } else if (options & Files) {
            // Only update files that got added to the list
            QSet<QString> newFileList = m_files.toSet();
            newFileList.subtract(oldFileList);
            filesToUpdate.append(newFileList.toList());
        }

        modelManager->updateProjectInfo(pinfo);
        m_codeModelFuture = modelManager->updateSourceFiles(filesToUpdate);
    }
}

QStringList PremakeProject::allIncludePaths() const
{
    QStringList paths;
    paths += m_includePaths;
    //paths += m_projectIncludePaths;
    paths.removeDuplicates();
    return paths;
}

//QStringList PremakeProject::projectIncludePaths() const
//{ return m_projectIncludePaths; }

QStringList PremakeProject::files() const
{ return m_files; }

QStringList PremakeProject::scriptDepends() const
{ return m_scriptDepends; }

QStringList PremakeProject::generated() const
{ return m_generated; }

QStringList PremakeProject::includePaths() const
{ return m_includePaths; }

void PremakeProject::setIncludePaths(const QStringList &includePaths)
{ m_includePaths = includePaths; }

QByteArray PremakeProject::defines() const
{ return m_defines; }

void PremakeProject::setToolChain(ToolChain *tc)
{
    if (m_toolChain == tc)
        return;

    m_toolChain = tc;
    refresh(Configuration);

    emit toolChainChanged(m_toolChain);
}

ToolChain *PremakeProject::toolChain() const
{
    return m_toolChain;
}

QString PremakeProject::displayName() const
{
    return m_projectName;
}

QString PremakeProject::id() const
{
    return QLatin1String(Constants::PREMAKEPROJECT_ID);
}

Core::IFile *PremakeProject::file() const
{
    return m_file;
}

IProjectManager *PremakeProject::projectManager() const
{
    return m_manager;
}

QList<Project *> PremakeProject::dependsOn()
{
    return QList<Project *>();
}

QList<BuildConfigWidget*> PremakeProject::subConfigWidgets()
{
    QList<BuildConfigWidget*> list;
    list << new BuildEnvironmentWidget;
    return list;
}

PremakeProjectNode *PremakeProject::rootProjectNode() const
{
    return m_rootNode;
}

QStringList PremakeProject::files(FilesMode fileMode) const
{
    Q_UNUSED(fileMode)
    return m_files; // ### TODO: handle generated files here.
}

QVariantMap PremakeProject::toMap() const
{
    QVariantMap map(Project::toMap());
    map.insert(QLatin1String(TOOLCHAIN_KEY), m_toolChain ? m_toolChain->id() : QString());
    return map;
}

bool PremakeProject::fromMap(const QVariantMap &map)
{
    if (!Project::fromMap(map))
        return false;

    // Sanity check: We need both a buildconfiguration and a runconfiguration!
    QList<Target *> targetList = targets();
    foreach (Target *t, targetList) {
        if (!t->activeBuildConfiguration()) {
            removeTarget(t);
            delete t;
            continue;
        }
        if (!t->activeRunConfiguration())
            t->addRunConfiguration(new CustomExecutableRunConfiguration(t));
    }

    // Add default setup:
    if (targets().isEmpty()) {
        PremakeTargetFactory *factory =
                ExtensionSystem::PluginManager::instance()->getObject<PremakeTargetFactory>();
        Target *t = factory->create(this, QLatin1String(PREMAKE_DESKTOP_TARGET_ID));

        QString id = map.value(QLatin1String(TOOLCHAIN_KEY)).toString();
        const ToolChainManager *toolChainManager = ToolChainManager::instance();

        if (!id.isNull()) {
            t->activeBuildConfiguration()->setToolChain(toolChainManager->findToolChain(id));
        } else {
            ProjectExplorer::Abi abi = ProjectExplorer::Abi::hostAbi();
            abi = ProjectExplorer::Abi(abi.architecture(), abi.os(),  ProjectExplorer::Abi::UnknownFlavor,
                                       abi.binaryFormat(), abi.wordWidth() == 32 ? 32 : 0);
            QList<ToolChain *> tcs = toolChainManager->findToolChains(abi);
            if (tcs.isEmpty())
                tcs = toolChainManager->toolChains();
            if (!tcs.isEmpty()) {
                foreach (ToolChain *tc, tcs) {
                    // We don't support MSVC yet
                    if (abiIsMsvc(tc->targetAbi()))
                        continue;

                    t->activeBuildConfiguration()->setToolChain(tc);
                }
            }
        }
        addTarget(t);
    }




    setIncludePaths(allIncludePaths());

    refresh(Everything);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
// PremakeProjectFile
////////////////////////////////////////////////////////////////////////////////////

PremakeProjectFile::PremakeProjectFile(PremakeProject *parent, QString fileName)
    : Core::IFile(parent),
      m_project(parent),
      m_fileName(fileName)
{ }

PremakeProjectFile::~PremakeProjectFile()
{ }

bool PremakeProjectFile::save(QString *errorString, const QString &fileName, bool autoSave)
{
    return false;
}

QString PremakeProjectFile::fileName() const
{
    return m_fileName;
}

QString PremakeProjectFile::defaultPath() const
{
    return QString();
}

QString PremakeProjectFile::suggestedFileName() const
{
    return QString();
}

QString PremakeProjectFile::mimeType() const
{
    return QLatin1String(Constants::PREMAKEMIMETYPE);
}

bool PremakeProjectFile::isModified() const
{
    return false;
}

bool PremakeProjectFile::isReadOnly() const
{
    return true;
}

bool PremakeProjectFile::isSaveAsAllowed() const
{
    return false;
}

void PremakeProjectFile::rename(const QString &newName)
{
    // Can't happen
    Q_UNUSED(newName);
    Q_ASSERT(false);
}

Core::IFile::ReloadBehavior PremakeProjectFile::reloadBehavior(ChangeTrigger state, ChangeType type) const
{
    Q_UNUSED(state)
    Q_UNUSED(type)
    return BehaviorSilent;
}

bool PremakeProjectFile::reload(QString *errorString, ReloadFlag flag, ChangeType type)
{
    Q_UNUSED(flag)
    Q_UNUSED(type)
    return false;
}
