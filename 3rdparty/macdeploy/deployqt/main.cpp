/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
**
** No Commercial Usage
** This file contains pre-release code and may only be used for
** evaluation and testing purposes.  It may not be used for commercial
** development.  You may use this file in accordance with the terms and
** conditions contained in the either Technology Preview License
** Agreement or the Beta Release License Agreement.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/
#include "../shared/shared.h"

int main(int argc, char **argv)
{
    QString appBundlePath;
    if (argc > 1)
        appBundlePath = QString::fromLocal8Bit(argv[1]);

    if (argc < 2 || appBundlePath.startsWith("-")) {
        qDebug() << "Usage: macdeployqt app-bundle [-no-plugins] [-dmg]";
        qDebug() << "";
        qDebug() << "macdeployqt creates self-contained application bundles that";
        qDebug() << "contains the Qt frameworks and plugins used by the application.";
        qDebug() << "";
        qDebug() << "Only frameworks in use are copied into the bundle.";
        qDebug() << "Plugins related to a framework are copied in with the";
        qDebug() << "framework. The accessibilty, image formats, and text codec";
        qDebug() << "plugins are always copied, unless \"-no-plugins\" is specified.";
        qDebug() << "";
        qDebug() << "See the \"Deploying an Application on Qt/Mac\" typic in the";
        qDebug() << "documentation for more information about deployment on Mac OS X.";

        return 0;
    }
    
    if (appBundlePath.endsWith("/"))
        appBundlePath.chop(1);
    
    DeploymentInfo deploymentInfo  = deployQtFrameworks(appBundlePath);
    
    bool plugins = true;
    bool dmg = false;
        
    for (int i = 2; i < argc; ++i) {
        QByteArray argument = QByteArray(argv[i]);
        if (argument == QByteArray("-no-plugins"))
            plugins = false;
        if (argument == QByteArray("-dmg"))
            dmg = true;
    }

    if (plugins) {
        if (deploymentInfo.qtPath.isEmpty())
            deploymentInfo.pluginPath = "/Developer/Applications/Qt/plugins"; // Assume binary package.
        else
            deploymentInfo.pluginPath = deploymentInfo.qtPath + "/plugins";

        qDebug() << "";
        qDebug() << "Deploying plugins from" << deploymentInfo.pluginPath;
        deployPlugins(appBundlePath, deploymentInfo);
        createQtConf(appBundlePath);
    }



    if (dmg) {
        qDebug() << "Creating disk image (.dmg) for" << appBundlePath;
        createDiskImage(appBundlePath);
    }
}

