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

#include "premakeprojectplugin.h"
#include "premakeprojectmanager.h"
#include "premakeprojectwizard.h"
#include "premakerunconfiguration.h"
#include "premakeprojectconstants.h"
#include "luaeditor.h"
#include "premakestep.h"
#include "makestep.h"
#include "premaketarget.h"

#include <coreplugin/icore.h>
#include <utils/mimetypes/mimedatabase.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>

#include <QtCore/QtPlugin>
#include <QtCore/QDebug>
#include <QtWidgets/QAction>

using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;

PremakeProjectPlugin::PremakeProjectPlugin()
    : m_luaEditorFactory(0)
{ }

PremakeProjectPlugin::~PremakeProjectPlugin()
{
    // FIXME: Qt5 no removeObject
    //removeObject(m_luaEditorFactory);
    delete m_luaEditorFactory;
}

bool PremakeProjectPlugin::initialize(const QStringList &, QString *errorMessage)
{
    using namespace Core;

    // FIXME: Qt5 check types regisytration. Core plugin has this code: "register all mime types from all plugins"
    // https://github.com/qt-creator/qt-creator/blob/299d34d54757823014db77bf0cc9ba864353731a/src/plugins/coreplugin/coreplugin.cpp#L133
    //ICore *core = ICore::instance();
    //Core::MimeDatabase *mimeDB = core->mimeDatabase();

    //const QLatin1String mimetypesXml(":/premakeproject/PremakeProject.mimetypes.xml");

    //if (! mimeDB->addMimeTypes(mimetypesXml, errorMessage))
    //    return false;

    PremakeManager *manager = new PremakeManager;

    m_luaEditorFactory = new LuaSupport::LuaEditorFactory(manager);
    addObject(m_luaEditorFactory);

    addAutoReleasedObject(manager);
    addAutoReleasedObject(new MakeStepFactory);
    addAutoReleasedObject(new PremakeStepFactory);
    addAutoReleasedObject(new PremakeTargetFactory);
    addAutoReleasedObject(new PremakeRunConfigurationFactory);
    // TODO: Uncomment when ready to use
//    addAutoReleasedObject(new PremakeProjectWizard);

    return true;
}

void PremakeProjectPlugin::extensionsInitialized()
{ }

Q_EXPORT_PLUGIN(PremakeProjectPlugin)
