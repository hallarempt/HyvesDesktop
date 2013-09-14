#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

#include "Version.h"

#define RELEASE_BUILD

const QString HD_PRODUCT_NAME("Hyves Desktop");
const QString HD_PRODUCT_URL("www.hyves.nl");
const QString HD_ORGANIZATION_NAME("Hyves");
const QString HD_DOMAIN_NAME("hyves.nl");
const Version HD_VERSION("1.3.4977");

// stats uploader:
const qint64 HD_STATS_MAX_FILESIZE(200 * 1024);	// do not upload more than 200kB data from log file
const QString HD_STATS_PROD_URL("http://oops.hyves.org/submit");
const QString HD_STATS_TEST_URL("http://localhost:8000/submit");
const QString HD_STATS_UPLOAD_SETTINGS( "General/Autostart;"
                                        "General/allowMultipleClients;"
                                        "General/enableLocalHistory;"
                                        "General/showOfflineFriends;"
                                        "Statistics/submitUsageStats;"
                                        "Notifications/newOnlineSound;"
                                        "Notifications/newSessionSound;"
                                        "Notifications/headlineSound;"
                                        "Notifications/newOnlineAlert;"
                                        "Notifications/newSessionAlert;"
                                        "Notifications/headlineAlert" );

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
