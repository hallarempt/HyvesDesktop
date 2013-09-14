#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

#include "Version.h"

#define RELEASE_BUILD

const QString HD_PRODUCT_NAME("Hyves Desktop");
const QString HD_PRODUCT_URL("www.hyves.nl");
const QString HD_ORGANIZATION_NAME("Hyves");
const QString HD_DOMAIN_NAME("hyves.nl");
const QString HD_VERSION("1.3.4977");
const QString HD_CRASH_URL("oops.hyves.org");

#ifdef Q_WS_MAC
	const QString HD_PLATFORM = "macosx";
#endif
#ifdef Q_WS_WIN
	const QString HD_PLATFORM = "win32";
#endif
#ifdef Q_WS_X11
	const QString HD_PLATFORM = "linux";
#endif

#endif // CONFIG_H
