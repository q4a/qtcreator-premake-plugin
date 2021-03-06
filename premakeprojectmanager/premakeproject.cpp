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
#include "luatocpp.h"

#include <projectexplorer/buildenvironmentwidget.h>
#include <projectexplorer/headerpath.h>
#include <projectexplorer/customexecutablerunconfiguration.h>
#include <projectexplorer/toolchainmanager.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <cpptools/cppmodelmanagerinterface.h>
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

PremakeProject::PremakeProject(const Utils::FileName &fileName)
    : Project(Constants::PREMAKEMIMETYPE, fileName)
{
    m_file = new PremakeProjectFile(this, fileName.toString());
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

static void projectParseError(const QString &errorMessage)
{
    //Core::ICore::instance()->messageManager()->printToOutputPanePopup(
    //        QCoreApplication::translate("PremakeProject", "Premake error: ") + errorMessage);
    const QString fullMessage = QCoreApplication::translate("PremakeProject", "Premake error: %1").arg(errorMessage);
    const Core::MessageManager::PrintToOutputPaneFlags flags = Core::MessageManager::ModeSwitch;
    Core::MessageManager::write(fullMessage, flags);
}

static void projectParseError(const char *errorMessage)
{
    projectParseError(QString::fromLocal8Bit(errorMessage));
}

static QString callLuaFunction(lua_State *L, const QByteArray &funcname,
                               const QList<QByteArray> &args = QList<QByteArray>())
{
    CallLuaFunctionSingleReturnValue callback(args);
    if (!luaRecursiveAccessor(L, funcname, callback)) {
        projectParseError(QString::fromLatin1(funcname) + QString::fromLatin1(": ")
                          + callback.error());
    }
    return callback.result();
}

static void tableToStringList(lua_State *L, const QByteArray &tablename, QStringList &to)
{
    GetStringList callback(to);
    if (!luaRecursiveAccessor(L, tablename, callback))
        projectParseError(callback.error());
//    qDebug() << Q_FUNC_INFO << tablename << "=" << to << endl;
}

static void tableToStringMapList(lua_State *L, const QByteArray &tablename, StringMapList &to)
{
    GetStringMapList callback(to);
    if (!luaRecursiveAccessor(L, tablename, callback))
        projectParseError(callback.error());
}


void PremakeProject::parseConfigurations()
{
    /// @todo Create a persistent lua state with all built-in scripts loaded
    /// and clone it before loading project
    lua_State *L = LuaManager::instance()->initLuaState(m_fileName, "_qtcreator", QByteArray(),
                                                        false, QLatin1String(""));
    if(call_premake_main(L) != 0)
        projectParseError(lua_tostring(L, -1));

    m_projectName = callLuaFunction(L, "_qtcreator_projectname");
    m_rootNode->setDisplayName(m_projectName);

    tableToStringList(L, QString::fromLatin1("premake.solution.list.%1.configurations")
                      .arg(m_projectName).toLocal8Bit(), m_configurations);

    lua_close(L);
}

void PremakeProject::parseProject(RefreshOptions options)
{
    const PremakeBuildConfiguration *conf = activeTarget()->activeBuildConfiguration();
    //QHash<QString,QString> qtInfo;
    //if (conf->qtVersion())
    //    qtInfo = conf->qtVersion()->versionInfo();

    /// @todo Create a persistent lua state with all built-in scripts loaded
    /// and clone it before loading project
    lua_State *L = LuaManager::instance()->initLuaState(m_fileName, "_qtcreator",
                                                        conf->internalConfigurationName(),
                                                        conf->shadowBuildEnabled(),
                                                        conf->buildDirectory());

    if(call_premake_main(L) != 0)
        projectParseError(lua_tostring(L, -1));

    if (options & ProjectName) {
        m_projectName = callLuaFunction(L, "_qtcreator_projectname");
        m_rootNode->setDisplayName(m_projectName);
    }

    if(options & RootPath) {
        const QString rootPath = callLuaFunction(L, "_qtcreator_rootpath");
        if (!rootPath.isEmpty())
            m_rootNode->setPath(rootPath);
    }

    if (options & Files) {
        tableToStringList(L, "_qtcreator_files", m_files);
        tableToStringList(L, "_qtcreator_generated_files", m_generated);
        tableToStringList(L, "_qtcreator_scriptdepends", m_scriptDepends);
        m_scriptDepends.removeDuplicates();
        foreach(const QString &file, m_files) {
            m_scriptDepends.removeAll(file);
        }
    }

    // if (options & ?) {
    tableToStringList(L, QString::fromLatin1("premake.solution.list.%1.configurations")
                      .arg(m_projectName).toLocal8Bit(), m_configurations);
    foreach (BuildConfiguration *c, activeTarget()->buildConfigurations()) {
        PremakeBuildConfiguration *pc = qobject_cast<PremakeBuildConfiguration *>(c);
        if (pc && pc->shortConfigurationName().isEmpty()) {
            const QString result = callLuaFunction(L, "premake.getconfigname",
                                          QList<QByteArray>() << pc->internalConfigurationName() << "Native" << "true");
            qDebug() << Q_FUNC_INFO << "Updating make config for" << pc->displayName() << result;
            pc->setShortConfigurationName(result.toLocal8Bit());

        }
    }

    StringMapList targets;
    tableToStringMapList(L, "_qtcreator_targets", targets);
    foreach (const StringMap &tgt, targets) {
        PremakeBuildTarget target;
        target.executable = tgt[QLatin1String("executablePath")];
        target.workingDirectory = tgt[QLatin1String("absoluteDirectory")];
        target.isConsole = (tgt[QLatin1String("isConsole")] == QLatin1String("true"));
        m_buildTargets[tgt[QLatin1String("title")]] = target;
    }

    // }

    if (options & Configuration) {
        tableToStringList(L, "_qtcreator_includes", m_includePaths);
        m_includePaths.removeDuplicates();
        for (int i=0; i<m_includePaths.size(); ++i) {
            m_includePaths[i].replace(QLatin1String("$(QT_INCLUDE)"),
                                      conf->qtVersion()->headerPath().toString()); // qtInfo.value(QLatin1String("QT_INSTALL_HEADERS")));
            m_includePaths[i].replace(QLatin1String("$(QT_LIB)"),
                                      conf->qtVersion()->libraryPath().toString()); // qtInfo.value(QLatin1String("QT_INSTALL_LIBS")));
        }
//        qDebug() << Q_FUNC_INFO << "m_includePaths=" << m_includePaths;

        QStringList defines;
        tableToStringList(L, "_qtcreator_defines", defines);
        defines.removeDuplicates();
        m_defines.clear();
        foreach(const QString &def, defines)
            m_defines += QString::fromLatin1("#define %1\n").arg(def)
                    .replace(QLatin1Char('='), QLatin1Char(' ')).toLocal8Bit();
//        qWarning() << m_defines;
    }

    if (options & Files)
        emit fileListChanged();

    emit buildTargetsChanged();

    lua_close(L);
}

void PremakeProject::refresh(RefreshOptions options)
{
// FIXME: Qt5 will port later
/*
    QSet<QString> oldFileList;
    if (!(options & Configuration))
        oldFileList = m_files.toSet();

    parseProject(options);

    if (options & Files)
        m_rootNode->refresh();

    CppTools::CppModelManagerInterface *modelManager =
        CppTools::CppModelManagerInterface::instance();

    if (modelManager) {
        CppTools::CppModelManagerInterface::ProjectInfo pinfo = modelManager->projectInfo(this);
        ToolChain *tc = activeTarget()->activeBuildConfiguration()->toolChain();

#if IDE_VER < IDE_VERSION_CHECK(2, 4, 80)
        pinfo.frameworkPaths.clear();
        pinfo.includePaths.clear();

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
        pinfo.sourceFiles = files();
#else
        pinfo.clearProjectParts();
        CPlusPlus::CppModelManagerInterface::ProjectPart::Ptr part(
                    new CPlusPlus::CppModelManagerInterface::ProjectPart);

        if (tc) {
            part->defines = tc->predefinedMacros(QStringList());
            part->defines += '\n';

            foreach (const HeaderPath &headerPath, tc->systemHeaderPaths()) {
                if (headerPath.kind() == HeaderPath::FrameworkHeaderPath)
                    part->frameworkPaths.append(headerPath.path());
                else
                    part->includePaths.append(headerPath.path());
            }
        }

        part->includePaths += allIncludePaths();
        part->defines += m_defines;
        part->sourceFiles = files();

        pinfo.appendProjectPart(part);
#endif

        QStringList filesToUpdate;

        if (options & Configuration) {
#if IDE_VER < IDE_VERSION_CHECK(2, 4, 80)
            filesToUpdate = pinfo.sourceFiles;
#else
            filesToUpdate = pinfo.sourceFiles();
#endif
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
*/
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

QStringList PremakeProject::configurations()
{
    if (m_configurations.isEmpty())
        parseConfigurations();

    return m_configurations;
}

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

bool PremakeProject::hasBuildTarget(const QString &title) const
{
    qDebug() << Q_FUNC_INFO;
    return m_buildTargets.contains(title);
}

PremakeBuildTarget PremakeProject::buildTargetForTitle(const QString &title)
{
    qDebug() << Q_FUNC_INFO;
    return m_buildTargets[title];
}

QStringList PremakeProject::buildTargetTitles() const
{
    qDebug() << Q_FUNC_INFO;
    return m_buildTargets.keys();
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

Core::IDocument *PremakeProject::document() const
{
    return m_file;
}

QObject *PremakeProject::projectManager() const
{
    return m_manager;
}

QList<Project *> PremakeProject::dependsOn()
{
    return QList<Project *>();
}

/*
QList<BuildConfigWidget*> PremakeProject::subConfigWidgets()
{
    QList<BuildConfigWidget*> list;
    list << new BuildEnvironmentWidget;
    return list;
}
*/

PremakeProjectNode *PremakeProject::rootProjectNode() const
{
    return m_rootNode;
}

// FIXME: Qt5 no FilesMode
//QStringList PremakeProject::files(FilesMode fileMode) const
//{
//    Q_UNUSED(fileMode)
//    return m_files; // ### TODO: handle generated files here.
//}

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
    : Core::IDocument(parent),
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

Core::IDocument::ReloadBehavior PremakeProjectFile::reloadBehavior(ChangeTrigger state, ChangeType type) const
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
