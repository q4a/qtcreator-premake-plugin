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

#include "premakeprojectwizard.h"

#include <coreplugin/icore.h>
#include <coreplugin/basefilewizard.h>
#include <utils/mimetypes//mimedatabase.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/customwizard/customwizard.h>

#include <utils/filewizardpage.h>

#include <QtGui/QIcon>

#include <QtWidgets/QApplication>
#include <QtWidgets/QStyle>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QtDebug>
#include <QtCore/QCoreApplication>

#include "premakeprojectconstants.h"

using namespace PremakeProjectManager::Internal;
using namespace Utils;

//////////////////////////////////////////////////////////////////////////////
// PremakeProjectWizardDialog
//////////////////////////////////////////////////////////////////////////////

PremakeProjectWizardDialog::PremakeProjectWizardDialog(QWidget *parent)
    : Utils::Wizard(parent)
{
    setWindowTitle(tr("Import Existing Project"));

    // first page
    m_firstPage = new FileWizardPage;
    m_firstPage->setTitle(tr("Project Name and Location"));
    m_firstPage->setFileNameLabel(tr("Project name:"));
    m_firstPage->setPathLabel(tr("Location:"));

    const int firstPageId = addPage(m_firstPage);
    wizardProgress()->item(firstPageId)->setTitle(tr("Location"));
}

PremakeProjectWizardDialog::~PremakeProjectWizardDialog()
{ }

QString PremakeProjectWizardDialog::path() const
{
    return m_firstPage->path();
}

void PremakeProjectWizardDialog::setPath(const QString &path)
{
    m_firstPage->setPath(path);
}

QString PremakeProjectWizardDialog::projectName() const
{
    return m_firstPage->fileName();
}

PremakeProjectWizard::PremakeProjectWizard(const Core::BaseFileWizardFactory *factory,
                                           QWidget *parent)
    : Core::BaseFileWizard(factory, QVariantMap(), parent)
{ }

PremakeProjectWizard::~PremakeProjectWizard()
{ }

Core::WizardDialogParameters PremakeProjectWizard::parameters()
{
// FIXME: Qt5 will port later
/*
    Core::WizardDialogParameters parameters(ProjectWizard);
    // TODO do something about the ugliness of standard icons in sizes different than 16, 32, 64, 128
    {
        QPixmap icon(22, 22);
        icon.fill(Qt::transparent);
        QPainter p(&icon);
        p.drawPixmap(0, 0, 22, 22, QPixmap(QLatin1String(Constants::ICON_PREMAKE)));
        parameters.setIcon(icon);
    }
    parameters.setDisplayName(tr("Premake Project"));
    parameters.setId(QLatin1String("Y.Premake"));
    parameters.setDescription(tr("New Premake project"));
    parameters.setCategory(QLatin1String(ProjectExplorer::Constants::PROJECT_WIZARD_CATEGORY));
#if IDE_VER >= IDE_VERSION_CHECK(2, 4, 80)
    parameters.setDisplayCategory(QCoreApplication::translate("ProjectExplorer", ProjectExplorer::Constants::PROJECT_WIZARD_CATEGORY_DISPLAY));
#else
    parameters.setDisplayCategory(QCoreApplication::translate("ProjectExplorer", ProjectExplorer::Constants::PROJECT_WIZARD_TR_CATEGORY));
#endif
    return parameters;
*/
}

QWizard *PremakeProjectWizard::createWizardDialog(QWidget *parent,
                                                  const QString &defaultPath,
                                                  const QList<QWizardPage *> &extensionPages) const
{
// FIXME: Qt5 will port later
/*
    PremakeProjectWizardDialog *wizard = new PremakeProjectWizardDialog(parent);
    setupWizard(wizard);

    wizard->setPath(defaultPath);

    foreach (QWizardPage *p, extensionPages)
        BaseFileWizard::applyExtensionPageShortTitle(wizard, wizard->addPage(p));

    return wizard;
*/
}

void PremakeProjectWizard::getFileList(const QDir &dir, const QString &projectRoot,
                                       const QStringList &suffixes,
                                       QStringList *files, QStringList *paths) const
{
    const QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files |
                                                         QDir::Dirs |
                                                         QDir::NoDotAndDotDot |
                                                         QDir::NoSymLinks);

    foreach (const QFileInfo &fileInfo, fileInfoList) {
        QString filePath = fileInfo.absoluteFilePath();
        filePath = filePath.mid(projectRoot.length() + 1);

        if (fileInfo.isDir() && isValidDir(fileInfo)) {
            getFileList(QDir(fileInfo.absoluteFilePath()), projectRoot,
                        suffixes, files, paths);

            if (! paths->contains(filePath))
                paths->append(filePath);
        }

        else if (suffixes.contains(fileInfo.suffix()))
            files->append(filePath);
    }
}

bool PremakeProjectWizard::isValidDir(const QFileInfo &fileInfo) const
{
    const QString fileName = fileInfo.fileName();
    const QString suffix = fileInfo.suffix();

    if (fileName.startsWith(QLatin1Char('.')))
        return false;

    else if (fileName == QLatin1String("CVS"))
        return false;

    // ### user include/exclude

    return true;
}

Core::GeneratedFiles PremakeProjectWizard::generateFiles(const QWizard *w,
                                                         QString *errorMessage) const
{
// FIXME: Qt5 will port later
/*
    Q_UNUSED(errorMessage)

    const PremakeProjectWizardDialog *wizard = qobject_cast<const PremakeProjectWizardDialog *>(w);
    const QString projectPath = wizard->path();
    const QDir dir(projectPath);
    const QString projectName = wizard->projectName();
    const QString creatorFileName = QFileInfo(dir, projectName + QLatin1String(".creator")).absoluteFilePath();
    const QString filesFileName = QFileInfo(dir, projectName + QLatin1String(".files")).absoluteFilePath();
    const QString includesFileName = QFileInfo(dir, projectName + QLatin1String(".includes")).absoluteFilePath();
    const QString configFileName = QFileInfo(dir, projectName + QLatin1String(".config")).absoluteFilePath();

    Core::ICore *core = Core::ICore::instance();
    Core::MimeDatabase *mimeDatabase = core->mimeDatabase();

    const QStringList suffixes = mimeDatabase->suffixes();

    QStringList sources, paths;
    getFileList(dir, projectPath, suffixes, &sources, &paths);

    Core::MimeType headerTy = mimeDatabase->findByType(QLatin1String("text/x-chdr"));

    QStringList nameFilters;
    foreach (const Core::MimeGlobPattern &gp, headerTy.globPatterns())
        nameFilters.append(gp.regExp().pattern());

    QStringList includePaths;
    foreach (const QString &path, paths) {
        QFileInfo fileInfo(dir, path);
        QDir thisDir(fileInfo.absoluteFilePath());

        if (! thisDir.entryList(nameFilters, QDir::Files).isEmpty())
            includePaths.append(path);
    }

    Core::GeneratedFile generatedCreatorFile(creatorFileName);
    generatedCreatorFile.setContents(QLatin1String("[General]\n"));
    generatedCreatorFile.setAttributes(Core::GeneratedFile::OpenProjectAttribute);

    Core::GeneratedFile generatedFilesFile(filesFileName);
    generatedFilesFile.setContents(sources.join(QLatin1String("\n")));

    Core::GeneratedFile generatedIncludesFile(includesFileName);
    generatedIncludesFile.setContents(includePaths.join(QLatin1String("\n")));

    Core::GeneratedFile generatedConfigFile(configFileName);
    generatedConfigFile.setContents(QLatin1String("// ADD PREDEFINED MACROS HERE!\n"));

    Core::GeneratedFiles files;
    files.append(generatedFilesFile);
    files.append(generatedIncludesFile);
    files.append(generatedConfigFile);
    files.append(generatedCreatorFile);

    return files;
*/
}

bool PremakeProjectWizard::postGenerateFiles(const QWizard *w, const Core::GeneratedFiles &l, QString *errorMessage)
{
    Q_UNUSED(w);
    return ProjectExplorer::CustomProjectWizard::postGenerateOpen(l, errorMessage);
}
