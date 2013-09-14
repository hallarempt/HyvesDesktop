PROJECT(HyvesDesktopLib)

ADD_DEFINITIONS(-DHYVESLIB_MAKEDLL)

IF(NO_BREAKPAD)
	ADD_DEFINITIONS(-DNO_BREAKPAD)
ENDIF(NO_BREAKPAD)

ADD_SUBDIRECTORY(tests)

IF (APPLE)
	INCLUDE_DIRECTORIES(/Developer/Headers/FlatCarbon)
	FIND_LIBRARY(CARBON_LIBRARY Carbon)
	FIND_LIBRARY(IOKIT_LIBRARY IOKit)
								    
  	FIND_PATH(Growl_INCLUDE_DIR Growl/Growl.h
    	${CMAKE_SOURCE_DIR}/3rdparty/
	)
	FIND_LIBRARY(Growl_LIBRARY
    	NAMES Growl
    	PATHS ${CMAKE_SOURCE_DIR}/3rdparty/
	)
	

	MARK_AS_ADVANCED(CARBON_LIBRARY)
	MARK_AS_ADVANCED(IOKIT_LIBRARY)
	SET(EXTRA_LIBS ${CARBON_LIBRARY} ${IOKIT_LIBRARY} ${Growl_LIBRARY})
ENDIF (APPLE)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/3rdparty/genusapis
	${CMAKE_SOURCE_DIR}/3rdparty/qtsingleapplication-2.6-opensource/src
	${CMAKE_SOURCE_DIR}/3rdparty/google-breakpad-read-only/src
	${CMAKE_SOURCE_DIR}/3rdparty/idle
	${QCA2_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/3rdparty/qjson/lib
	${CMAKE_SOURCE_DIR}/3rdparty/quazip
	${CMAKE_SOURCE_DIR}/src
)

SET(SOURCES
	contextmenumanager/ContextMenuManager.cpp
	extender/Environment.cpp
	extender/Extender.cpp
	extender/JSSlotWrapper.cpp
	extender/MetaTypes.cpp
	filer/Filer.cpp
	historykeeper/HistoryKeeper.cpp
	hyvesapi/HyvesAPI.cpp
	informer/Informer.cpp
	jabber/DelayedJabberMessage.cpp
	jabber/Jabber.cpp
	jabber/JabberMock.cpp
	logger/Logger.cpp
	logger/JSLogger.cpp
	logger/UsageStatsUploader.cpp
	mediacache/MediaCache.cpp
	mediacache/NetworkReply.cpp
	player/Player.cpp
	plugger/CXXPluginLoader.cpp
	plugger/Plugger.cpp
	plugger/PluginInfo.cpp
	plugger/PluginLoader.cpp
	richtext/Label.cpp
	richtext/TextBrowser.cpp
	richtext/TextDocument.cpp
	richtext/TextEdit.cpp
	roster/Roster.cpp
	roster/RosterItem.cpp
	settingsmanager/SettingsManager.cpp
	systemspy/SimpleConnectionChecker.cpp
	systemspy/SystemSpy.cpp
	systemtrayicon/SystemTrayIcon.cpp
	tester/Tester.cpp
	tooltipper/ContactToolTip.cpp
	tooltipper/ToolTipper.cpp
	translator/Translator.cpp
	updater/Updater.cpp
        windowmanager/MainWindowBase.cpp
	windowmanager/ScriptWindow.cpp
	windowmanager/SliderWindow.cpp
	windowmanager/WindowBase.cpp
	windowmanager/WindowInfo.cpp
	windowmanager/WindowManager.cpp
	zipper/Zipper.cpp
	zipper/ZipCache.cpp
	AboutDialog.cpp
	Avatar.cpp
	CookieJar.cpp
	#DiskCache.cpp
	HyvesDesktopApplication.cpp
	LoadingWidget.cpp
	NetworkAccessManager.cpp
	NetworkReply.cpp
	UrlFetcher.cpp
	Util.cpp
	Version.cpp
	WebPage.cpp
	WebPluginFactory.cpp
	WebView.cpp
	WideSplitter.cpp
)

IF (NOT BUILD_64BIT)
set (SOURCES ${SOURCES}
	CrashHandler.cpp
)
ENDIF (NOT BUILD_64BIT)

SET(HEADERS
	config.h
	contextmenumanager/ContextMenuManager.h
	extender/Environment.h
	extender/Extender.h
	extender/JSSlotWrapper.h
	extender/MetaTypes.h
	filer/Filer.h
	historykeeper/HistoryKeeper.h
	hyvesapi/HyvesAPI.h
	informer/Informer.h
	jabber/DelayedJabberMessage.h
	jabber/Jabber.h
	jabber/JabberMock.h
	logger/Logger.h
	logger/JSLogger.h
	logger/UsageStatsUploader.h
	mediacache/MediaCache.h
	mediacache/NetworkReply.h
	player/Player.h
	plugger/CXXPluginInterface.h
	plugger/CXXPluginLoader.h
	plugger/Plugger.h
	plugger/PluginFile.h
	plugger/PluginInfo.h
	plugger/PluginLoader.h
	richtext/Label.h
	richtext/TextBrowser.h
	richtext/TextDocument.h
	richtext/TextEdit.h
	roster/Roster.h
	roster/RosterItem.h
	settingsmanager/SettingsManager.h
	systemspy/SimpleConnectionChecker.h
	systemspy/SystemSpy.h
	systemtrayicon/SystemTrayIcon.h
	tester/Tester.h
	tooltipper/ContactToolTip.h
	tooltipper/ToolTipper.h
	translator/Translator.h
	updater/Updater.h
	windowmanager/MainWindowBase.h
	windowmanager/ScriptWindow.h
	windowmanager/SliderWindow.h
	windowmanager/WindowBase.h
	windowmanager/WindowInfo.h
	windowmanager/WindowManager.h
	zipper/Zipper.h
	zipper/ZipCache.h
	AboutDialog.h
	Avatar.h
	CookieJar.h
	#DiskCache.h
	HyvesDesktopApplication.h
	LoadingWidget.h
	NetworkAccessManager.h
	NetworkReply.h
	UrlFetcher.h
	Util.h
	Version.h
	WebPage.h
	WebPluginFactory.h
	WebView.h
	WideSplitter.h
)

IF(UNIX)
	
	IF(APPLE)
		SET(SOURCES ${SOURCES} 
			LoginItemsAE.c 
			systemspy/SystemSpyOsx.cpp
			windowmanager/GrowlNotifier.cpp
		)
		SET(HEADERS ${HEADERS} 
			systemspy/SystemSpyOsx.h
			windowmanager/GrowlNotifier.h
		)
	ELSE(APPLE)
		SET(SOURCES ${SOURCES} 
			systemspy/SystemSpyUnix.cpp
		)
		SET(HEADERS ${HEADERS}
			systemspy/SystemSpyUnix.h
		)
	ENDIF(APPLE)
ENDIF(UNIX)
IF(WIN32)

	SET(SOURCES ${SOURCES}
		systemspy/SystemSpyWin.cpp
		${CMAKE_SOURCE_DIR}/HyvesDesktop.rc)

	SET(HEADERS ${HEADERS} 
		systemspy/SystemSpyWin.h
	)
ENDIF(WIN32)

SET(FORMS
)

# Qt wrappers
QT4_WRAP_UI(UI_HEADERS ${FORMS})

# Library
KDE4_ADD_LIBRARY(HyvesDesktopLib SHARED
	${SOURCES} ${HEADERS} ${UI_HEADERS}
)

IF (NOT BUILD_64BIT)
set (BREAKPAD_LIBS 
        breakpad
)
ENDIF (NOT BUILD_64BIT)



TARGET_LINK_LIBRARIES(HyvesDesktopLib qtsingleapplication genusapis ${BREAKPAD_LIBS} idle ${QCA2_LIBRARIES} ${ZLIB_LIBRARIES} qjson quazip ${QT_LIBRARIES} ${EXTRA_LIBS}) 

#TARGET_LINK_LIBRARIES(HyvesDesktopLib LINK_INTERFACE_LIBRARIES qtsingleapplication genusapis ${BREAKPAD_LIBS} idle ${QCA2_LIBRARIES} ${ZLIB_LIBRARIES} qjson quazip ${QT_LIBRARIES} ${EXTRA_LIBS}) 

IF(WIN32)
    INSTALL(TARGETS HyvesDesktopLib RUNTIME DESTINATION bin)
ELSE(WIN32)
    INSTALL(TARGETS HyvesDesktopLib LIBRARY DESTINATION bin)
ENDIF(WIN32)

IF(APPLE)
       ADD_CUSTOM_COMMAND(TARGET HyvesDesktopLib
                POST_BUILD 
		COMMAND ${CMAKE_COMMAND} -E make_directory "HyvesDesktop.app/Contents/Frameworks"
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/3rdparty/Growl.framework "HyvesDesktop.app/Contents/Frameworks/Growl.framework"
		COMMAND ${CMAKE_COMMAND} -E copy "src/libHyvesDesktopLib.dylib" "HyvesDesktop.app/Contents/Frameworks/"
                COMMAND "${QT_BINARY_DIR}/fixupplugins" "HyvesDesktop.app"  HyvesDesktop.app/Contents/Frameworks/libHyvesDesktopLib.dylib
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
       )
ENDIF(APPLE)
