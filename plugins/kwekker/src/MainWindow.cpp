/*
 * Hyves Desktop, Copyright (C) 2008-2009 Hyves (Startphone Ltd.)
 * http://www.hyves.nl/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  US
 */

#include <QAction>
#include <QEvent>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QProcess>
#include <QResource>
#include <QStackedWidget>
#include <QStatusBar>
#include <QWebFrame>
#include <QWebView>

#include "config.h"
#include "extender/Extender.h"
#include "jabber/Jabber.h"
#include "logger/Logger.h"
#include "plugger/Plugger.h"
#include "settingsmanager/SettingsManager.h"
#include "systemspy/SystemSpy.h"
#include "systemtrayicon/SystemTrayIcon.h"
#include "tester/Tester.h"
#include "tooltipper/ToolTipper.h"
#include "windowmanager/WindowManager.h"
#include "zipper/Zipper.h"
#include "AboutDialog.h"
#include "HyvesDesktopApplication.h"
#include "LoadingWidget.h"
#include "MainWindow.h"
#include "NetworkAccessManager.h"
#include "SettingsDialog.h"
#include "Util.h"
#include "WebPage.h"
#include "WebView.h"
#include "WideSplitter.h"

#include "ui_MainWindow.h"

namespace Kwekker {

const int defaultNewsViewHeight = 284;
// defines how often the introduction page is shown to the user
// note: this must be set accordingly in statics/intro/landingpagecounter.js
const int landingPageCounterMax = 5;

struct MainWindow::Private {
	Logger::Logger *log;
	
	Ui::MainWindow ui;
	
	QUrl baseUrl;
	QUrl newsUrl;
	
	QStackedWidget *widgetStack;
	WideSplitter *splitter;
	WebView *webView;
	WebView *newsWebView;
	LoadingWidget *loadingWidget;
	
	int newsWindowId;
	
	ToolTipper::ToolTipper *toolTipper;
	
	QAction *newWwwAction;
	QAction *newPhotosAction;
	QAction *newOtherAction;
	
	int numTestRuns;
	int totalNumFailedTests;
	
	int numNotifications;
	QLabel *notificationLabel;
	
	int reconnectSeconds;
	
	Private() :
		log(0),
		splitter(0),
		webView(0),
		newsWebView(0),
		newsWindowId(0),
		toolTipper(0),
		numTestRuns(0),
		totalNumFailedTests(0),
		numNotifications(0),
		notificationLabel(0),
		reconnectSeconds(0) {
	}
	
	~Private() {
		delete log;
	}
};

MainWindow::MainWindow() :
	WindowManager::MainWindowBase(tr("&Chat")),
	m_d(new Private()) {
	
	setObjectName("Kwekker");
	m_d->log = new Logger::Logger("KwekkerMainWindow");
	
	WindowManager::WindowManager::instance()->setOpenerWindowId(windowId());
	
	m_d->ui.setupUi(this);
	
	QLabel *photoUploaderLabel = new QLabel();
	photoUploaderLabel->setText(QString("<a href=\"#\" style=\"color: #000; text-decoration: none\"><img src=\":/data/photo.png\"> <strong>%1</strong> </a>").arg(tr("Photo Uploader")));
	statusBar()->addWidget(photoUploaderLabel);
	connect(photoUploaderLabel, SIGNAL(linkActivated(QString)), SLOT(launchPhotoUploader()));
	
	QMenu *addMenu = MainWindowBase::menu(tr("&Add"));
	m_d->newWwwAction = addMenu->addAction(tr("New &WWW..."), this, SLOT(newWwwClicked()));
	m_d->newPhotosAction = addMenu->addAction(tr("Upload &Photos..."), this, SLOT(uploadPhotosClicked()));
	m_d->newOtherAction = addMenu->addAction(tr("&Add blogs, gadgets, etc. on the website..."), this, SLOT(newOtherClicked()));
	
	m_d->newWwwAction->setEnabled(false);
	
	QMenu *toolMenu = MainWindowBase::menu(tr("&Tools"));
	QAction *reloadAction = toolMenu->addAction(tr("&Reload Kwekker"), this, SLOT(reloadKwekker()));
	QAction *removeBuzzAction = toolMenu->addAction(tr("Remove &Buzz"), this, SLOT(removeNewsWebView()));
	
#ifdef RELEASE_BUILD
	reloadAction->setVisible(false);
	removeBuzzAction->setVisible(false);
#else
	Q_UNUSED(reloadAction);
	Q_UNUSED(removeBuzzAction);
#endif
	
	QAction *preferencesAction = toolMenu->addAction(tr("&Settings"), this, SLOT(settings()));
	preferencesAction->setMenuRole(QAction::PreferencesRole);
	
	loadConfig();
	
	QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
	
	m_d->widgetStack = new QStackedWidget(this);
	setCentralWidget(m_d->widgetStack);
	
	m_d->loadingWidget = new LoadingWidget();
	
	m_d->webView = new WebView(this);
	m_d->webView->setMinimumSize(QSize(300, 300));
	m_d->webView->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
	m_d->webView->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
	m_d->webView->setJsForReload("mainWindow.reloadKwekker()");
	
	m_d->splitter = new WideSplitter();
	m_d->splitter->setOrientation(Qt::Vertical);
	m_d->splitter->addWidget(m_d->webView);
	m_d->splitter->setCollapsible(0, false);
	m_d->splitter->setHandleWidth(6);
	
	m_d->widgetStack->addWidget(m_d->loadingWidget);
	m_d->widgetStack->addWidget(m_d->splitter);
	
	connect(m_d->webView, SIGNAL(loadStarted()), SLOT(webViewLoadStarted()));
	connect(m_d->webView, SIGNAL(loadFinished(bool)), SLOT(webViewLoadFinished(bool)));
	
	connect(Jabber::Jabber::instance(), SIGNAL(connected()), SLOT(loggedIn()));
	connect(Jabber::Jabber::instance(), SIGNAL(disconnected()), SLOT(loggedOut()));
	connect(Jabber::Jabber::instance(), SIGNAL(resourceConflict()), SLOT(loggedInFromOtherLocation()));
	
	connect(SystemSpy::SystemSpy::instance(), SIGNAL(justAwoken()), SLOT(reloadKwekker()));
	
	connect(WindowManager::WindowManager::instance(), SIGNAL(eventRaised(QVariantMap)), SLOT(eventRaised(QVariantMap)));
	
	Extender::Extender::instance()->registerObject("mainWindow", this);
	
	m_d->toolTipper = new ToolTipper::ToolTipper(m_d->webView, this);
	
	// by default the landing page is shown 5 (landingPageCounterMax) times unless the "Show this intro again"-checkbox is cleared by the user
	int landingPageCounter = SettingsManager::SettingsManager::instance()->intValue("landingPageCounter", landingPageCounterMax);
	if (landingPageCounter > 0 || !SettingsManager::SettingsManager::instance()->boolValue("Statistics/UsageStatsNotificationDisplayed", false)) {
		SettingsManager::SettingsManager::instance()->setValue("landingPageCounter", --landingPageCounter);
		QTimer::singleShot(1000, this, SLOT(showLandingPage()));
	}
	
	if (Tester::Tester::isTesting()) {
		Tester::Tester *tester = Tester::Tester::instance();
		connect(tester, SIGNAL(testsFinished()), SLOT(testsFinished()));
		tester->setTestPage(m_d->webView->page());
		tester->scheduleTestFile(":/src/tests/InitializationTest.js");
		tester->scheduleTestFile(":/src/tests/LoginTest.js");
		tester->scheduleTestFile(":/src/tests/SearchBarTest.js");
		tester->scheduleTestFile(":/src/tests/NewSessionTest.js");
		tester->scheduleTestFile(":/src/tests/ReceiptConfirmationTest.js");
		//tester->scheduleTestFile(":/src/tests/UnifiedMessagingTest.js");
		tester->scheduleTestFile(":/src/tests/HistoryTest.js");
		tester->scheduleTestFile(":/src/tests/MUCSessionTest.js");
		tester->scheduleTestFile(":/src/tests/RosterTest.js");
		tester->runAllTests();
	}
	
	reloadKwekker();
	installEventFilter(this);
}

MainWindow::~MainWindow() {
	
	removeNewsWebView();
	saveConfig();
	
	Extender::Extender::instance()->unregisterObject("mainWindow");
	delete m_d;
}

QWebFrame *MainWindow::mainFrame() const {
	
	return m_d->webView->page()->mainFrame();
}

int MainWindow::windowId() const {
	
	return WindowBase::windowId();
}

bool MainWindow::isNewsFrameVisible() const {
	
	return isVisible() && m_d->newsWebView && m_d->splitter->sizes()[1] > 0;
}

void MainWindow::loadNewsComponent(QString component) {
	
	SettingsManager::SettingsManager *settingsManager = SettingsManager::SettingsManager::instance();
	
	if (component.isEmpty()) {
		component = settingsManager->stringValue("Buzz/component", "overview");
	}
	
	m_d->newsWebView->load(m_d->newsUrl.toString() + "&component=" + component);
	
	settingsManager->setValue("Buzz/component", component);
}

void MainWindow::setCursor(const QString &cursor) {
	
	if (cursor == "default") {
		unsetCursor();
	} else if (cursor == "wait") {
		QMainWindow::setCursor(QCursor(Qt::WaitCursor));
	}
}

void MainWindow::saveConfig() const {
	
	saveWindowState();
}

void MainWindow::loadConfig() {
	
	restoreWindowState();
	
	SettingsManager::SettingsManager *settingsManager = SettingsManager::SettingsManager::instance();
	m_d->baseUrl = "http://" + settingsManager->baseServer() + "/?module=Chat&action=showMain&mode=4";
	m_d->newsUrl = "http://" + settingsManager->baseServer() + "/?module=DesktopBuzz&action=showBuzz";
	
}

void MainWindow::loggedIn() {
	
	m_d->newWwwAction->setEnabled(true);
	
	addNewsWebView();
}

void MainWindow::loggedOut() {
	
	m_d->newWwwAction->setEnabled(false);
	
	removeNewsWebView();
	
	SystemTrayIcon::SystemTrayIcon::instance()->removePluginActions();
	WindowManager::WindowManager::instance()->closeAllWindows();
}

void MainWindow::loggedInFromOtherLocation() {
	
	loggedOut();
	
	m_d->webView->displayErrorPage(
		tr("Disconnected"),
		QString("<p style=\"padding: 10px\">%1</p>"
		        "<p style=\"padding: 0px 10px\"><button class=\"large\" onclick=\"reload()\"><span>%6</span></button></p>")
		.arg(tr("You have been disconnected because you logged in from another location."))
		.arg(tr("Reconnect"))
	);
}

void MainWindow::signOut() {
	
	saveConfig();
	removeNewsWebView();
	m_d->webView->page()->mainFrame()->evaluateJavaScript("logout()");
}

void MainWindow::resetReconnectSeconds() {
	
	m_d->reconnectSeconds = 0;
}

int MainWindow::reconnectSeconds() {
	
	switch (m_d->reconnectSeconds) {
		case 0:
			m_d->reconnectSeconds = 30;
			return m_d->reconnectSeconds;
		case 30:
			m_d->reconnectSeconds = 60;
			return m_d->reconnectSeconds;
		case 60:
			m_d->reconnectSeconds = 120;
			return m_d->reconnectSeconds;
		default:
			return 0;
	}
}

void MainWindow::reloadKwekker() {
	
	Jabber::Jabber::instance()->signOut(); // make sure we're logged out before (re)loading
	
	m_d->widgetStack->setCurrentWidget(m_d->loadingWidget);
	
	removeNewsWebView();
	
	QUrl baseUrl(m_d->baseUrl);
	if (SettingsManager::SettingsManager::instance()->boolValue("firstrun", true)) {
		baseUrl.addQueryItem("firstrun", "1");
		SettingsManager::SettingsManager::instance()->setValue("firstrun", false, true);
	}
	
	m_d->webView->load(baseUrl);
}

void MainWindow::settings() {
	
	SettingsDialog settingsDialog(this);
	settingsDialog.exec();
}

int MainWindow::numNotifications() const {
	
	return m_d->numNotifications;
}

void MainWindow::setNumNotifications(int numNotifications) {
	
	m_d->numNotifications = numNotifications;
	
	if (!m_d->notificationLabel) {
		return;
	}
	
	if (m_d->numNotifications == 0) {
		m_d->notificationLabel->setText("");
		SystemTrayIcon::SystemTrayIcon::instance()->setIcon("active");
	} else {
		m_d->notificationLabel->setText(QString("<a href=\"#\" style=\"color: #000; text-decoration: none\"><img src=\":/data/bell.png\"> <b>%1</b></a>").arg(m_d->numNotifications));
		SystemTrayIcon::SystemTrayIcon::instance()->setIcon("highlight");
	}
}

void MainWindow::toggleNotificationView() {
	
	if (m_d->newsWebView) {
		QList<int> sizes = m_d->splitter->sizes();
		if (sizes.size() == 2) {
			if (sizes[1] < 5) {
				sizes[1] = defaultNewsViewHeight;
				m_d->splitter->setSizes(sizes);
			}
		}
		
		m_d->newsWebView->page()->mainFrame()->evaluateJavaScript("window.buzzManager.toggleNotificationView()");
		m_d->newsWebView->update();
	}
}

void MainWindow::launchPhotoUploader() {
	
	m_d->log->usage("PhotoUploaderButtonClicked");
	Plugger::Plugger::instance()->runPlugin("photouploader");
}

void MainWindow::newWwwClicked() {
	
	m_d->log->usage("NewWwwMenuItemClicked");
	m_d->webView->page()->mainFrame()->evaluateJavaScript("showInputWWW()");
}

void MainWindow::uploadPhotosClicked() {
	
	m_d->log->usage("UploadPhotosMenuItemClicked");
	Plugger::Plugger::instance()->runPlugin("photouploader");
}

void MainWindow::newOtherClicked() {
	
	m_d->log->usage("NewOtherMenuItemClicked");
	SettingsManager::SettingsManager *settingsManager = SettingsManager::SettingsManager::instance();
	QDesktopServices::openUrl("http://www." + settingsManager->baseServer() + "/profielbeheer/toevoegen");
}

void MainWindow::setFakeBuzzHtml(const QString &dataFileName) {
	
	QString destinationPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/news.html";
	
	if (Zipper::copyFileFromArchive("bundle.zip", "/statics/test/" + dataFileName, destinationPath)) {
		return;
	}
	
	m_d->log->notice("Couldn't copy file from 'bundle.zip'. Falling back to bundle directory...");
	
	QString bundlePath = "bundle/statics/test/" + dataFileName;
	if (QFile::exists(destinationPath) && !QFile::remove(destinationPath)) {
		m_d->log->warning(QString("Old file exists and could not be deleted: %1").arg(destinationPath));
		return;
	}
	if (!QFile::copy(bundlePath, destinationPath)) {
		m_d->log->warning(QString("Failed to copy file: %1 -> %2").arg(bundlePath).arg(destinationPath));
	}
}

void MainWindow::clearFakeBuzzHtml() {
	
	QString path = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/news.html";
	
	if (QFile::exists(path)) {
		bool removed = QFile::remove(path); 
		if (!removed) {
			m_d->log->warning(QString("Couldn't remove file %1.").arg(path));
		}
	}
}

void MainWindow::closeEvent(QCloseEvent *event) {
	
	saveWindowState();
	hide();
	event->accept();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
	
	saveWindowState();
}

void MainWindow::showEvent(QShowEvent *event) {
	
	QTimer::singleShot(500, this, SLOT(setFrameDecorationGeometry()));
}

void MainWindow::changeEvent(QEvent *event) {
	
	if (event->type() == QEvent::LanguageChange) {
		m_d->ui.retranslateUi(this);
	} else {
		QMainWindow::changeEvent(event);
	}
}

bool MainWindow::eventFilter(QObject *object, QEvent *event) {
	
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		switch (keyEvent->key()) {
			case Qt::Key_Slash:
				// '/'-key -> move focus to searchbar
				m_d->webView->setFocus(Qt::ShortcutFocusReason);
				m_d->webView->page()->mainFrame()->evaluateJavaScript("moveFocusToSearchbar()");
				return true;
				
			default:
				break;
		}
	}
	
	return QObject::eventFilter(object, event);
}

void MainWindow::eventRaised(const QVariantMap &event) {
	
	if (event["name"] == "windowClose" && event["class"] == "buzz") {
		m_d->log->usage("HyvesMiniClosed");
		m_d->newsWindowId = 0;
	}
}

void MainWindow::setFrameDecorationGeometry() {
	
	QRect inner = geometry();
	QRect outer = frameGeometry();
	int titleBarHeight = inner.y() - outer.y();
	int frameWidth = inner.x() - outer.x();
	int frameHeight = outer.height() - (inner.height() + titleBarHeight);
	WindowManager::WindowManager::instance()->setFrameDecorationGeometry(titleBarHeight, frameWidth, frameHeight);
}

void MainWindow::webViewLoadStarted() {
}

void MainWindow::webViewLoadFinished(bool ok) {
	
	m_d->webView->page()->view()->setAcceptDrops(false);
	m_d->widgetStack->setCurrentWidget(m_d->splitter);
}

void MainWindow::saveWindowState() const {
	
	SettingsManager::SettingsManager::instance()->setVariantValue("MainWindow/pos", pos());
	SettingsManager::SettingsManager::instance()->setVariantValue("MainWindow/size", size());
}

void MainWindow::restoreWindowState() {
	
	QDesktopWidget *desktop = qApp->desktop();
	QRect availableGeometry = desktop->availableGeometry(this);
	QRect screenGeometry = desktop->screenGeometry(this);
	
	int defaultWidth = availableGeometry.width() / 5;
	if (availableGeometry.width() > 1600) {
		defaultWidth = availableGeometry.width() / 6;
	}
	
#ifdef Q_WS_WIN
	int diffHeight = screenGeometry.height() - availableGeometry.height();
	QPoint defaultPos = QPoint(availableGeometry.x() + availableGeometry.width() - (defaultWidth + 4), availableGeometry.y());
	QSize defaultSize = QSize(defaultWidth, availableGeometry.height() - (availableGeometry.y() + diffHeight));
#else
#if QT_VERSION == 0x040600
#ifdef Q_OS_MAC 
	
	QRect inner = geometry();
	QRect outer = frameGeometry();
	int titleBarHeight = inner.y() - outer.y();
	QPoint defaultPos = QPoint(availableGeometry.x() + availableGeometry.width() - defaultWidth, 
				   screenGeometry.height() - availableGeometry.height());
	QSize defaultSize = QSize(defaultWidth, availableGeometry.height() - titleBarHeight);
	
#endif
#else
	QPoint defaultPos = QPoint(availableGeometry.x() + availableGeometry.width() - defaultWidth, availableGeometry.y());
	QSize defaultSize = QSize(defaultWidth, availableGeometry.height() - availableGeometry.y());
#endif	
#endif
	
	QPoint pos = SettingsManager::SettingsManager::instance()->variantValue("MainWindow/pos", defaultPos).toPoint();
	QSize size = SettingsManager::SettingsManager::instance()->variantValue("MainWindow/size", defaultSize).toSize();
	
	if (size.height() > defaultSize.height()) {
		size.setHeight(defaultSize.height());
	}
	
	if (desktop->numScreens() == 1) {
		if (!desktop->screenGeometry().contains(QRect(pos, size))) {
			pos = defaultPos;
			size = defaultSize;
		}
	} else {
		// Don't do anything for now. I'm not even sure Apple does the right thing
		// when you disconnect a monitor.
	}
	
	resize(size);
	move(pos);
	m_d->log->usage(QString("MainWindow restored on %1%2 screen %3/%4 (screen resolution: %5x%6)")
					.arg(desktop->isVirtualDesktop() ? "virtual " : "")
					.arg(desktop->primaryScreen() == desktop->screenNumber() ? "primary" : "secondary")
					.arg(desktop->screenNumber()).arg(desktop->numScreens())
					.arg(desktop->screenGeometry(desktop->screenNumber()).width())
					.arg(desktop->screenGeometry(desktop->screenNumber()).height()));
}

void MainWindow::newsLinkClicked(const QUrl &url) {
	m_d->log->debug("newsLinkClicked(" + url.toString() + ")");
	
	WindowManager::WindowManager *windowManager = WindowManager::WindowManager::instance();
	
	QString urlString = url.toString();
	if (!urlString.contains(SettingsManager::SettingsManager::instance()->baseServer()) ||
	    (!urlString.contains("/mini/") && !urlString.contains("/redirect/") && !urlString.contains("?l1=mo"))) {
		QDesktopServices::openUrl(url);
		return;
	}
	
	if (m_d->newsWindowId) {
		windowManager->windowExecute(m_d->newsWindowId, "bringToFront");
	} else {
		m_d->log->usage("HyvesMiniOpened");
		
		WindowManager::WindowInfo windowInfo = windowManager->windowInfo(windowId());
		
		QVariantMap arguments;
		arguments["caption"] = "Hyves";
		arguments["visible"] = true;
		arguments["width"] = 400 + 2 * windowManager->frameWidth();
		arguments["height"] = windowInfo.height();
		arguments["left"] = windowInfo.left() - arguments["width"].toInt();
		arguments["top"] = windowInfo.top();
		arguments["openLinksInUserBrowser"] = false;
		
		m_d->newsWindowId = windowManager->createWindow("buzz", Util::variantMapToJSON(arguments));
	}
	
	windowManager->windowExecute(m_d->newsWindowId, "navigate", "url=" + url.toString());
}

void MainWindow::splitterDoubleClicked() {
	
	QList<int> sizes = m_d->splitter->sizes();
	if (sizes.size() < 2) {
		return;
	}
	
	if (sizes[1] < 5) {
		sizes[1] = defaultNewsViewHeight;
	} else {
		sizes[1] = 0;
	}
	m_d->splitter->setSizes(sizes);
}

void MainWindow::addNewsWebView() {
	
	if (!m_d->newsWebView) {
		m_d->newsWebView = new WebView(this);
		
		WebPage *webPage = qobject_cast<WebPage *>(m_d->newsWebView->page());
		Q_ASSERT(webPage);
		webPage->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
		webPage->setOpenLinksInUserBrowser(false);
		webPage->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);
		connect(webPage, SIGNAL(linkClicked(QUrl)), this, SLOT(newsLinkClicked(QUrl)));
		
#ifdef RELEASE_BUILD
		// Hide the inspect/reload/stop etc. context menu
		m_d->newsWebView->setContextMenuPolicy(Qt::NoContextMenu);
#endif
		m_d->newsWebView->setMinimumSize(QSize(250, defaultNewsViewHeight));
		m_d->newsWebView->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
		
		m_d->splitter->addWidget(m_d->newsWebView);
		
		QVariant splitterSizes = SettingsManager::SettingsManager::instance()->variantValue("MainWindow/splitterSizes");
		if (splitterSizes.isValid()) {
			m_d->splitter->restoreState(splitterSizes.toByteArray());
		} else {
			QList<int> sizes;
			sizes << (m_d->widgetStack->height() - defaultNewsViewHeight) << defaultNewsViewHeight;
			m_d->splitter->setSizes(sizes);
		}
		
		m_d->newsWebView->setAcceptDrops(false);
		
		connect(m_d->splitter->handle(1), SIGNAL(doubleClicked()), SLOT(splitterDoubleClicked()));
		
		loadNewsComponent();
	}
	
	if (!m_d->notificationLabel) {
		m_d->notificationLabel = new QLabel();
		statusBar()->addPermanentWidget(m_d->notificationLabel);
		connect(m_d->notificationLabel, SIGNAL(linkActivated(QString)), SLOT(toggleNotificationView()));
	}
}

void MainWindow::removeNewsWebView() {
	
	if (m_d->newsWebView) {
		SettingsManager::SettingsManager::instance()->setVariantValue("MainWindow/splitterSizes", m_d->splitter->saveState());
		
		delete m_d->newsWebView;
		m_d->newsWebView = 0;
	}
	
	if (m_d->notificationLabel) {
		delete m_d->notificationLabel;
		m_d->notificationLabel = 0;
	}
}

void MainWindow::testsFinished() {
	
	Tester::Tester *tester = Tester::Tester::instance();
	m_d->totalNumFailedTests += tester->numFailedTests();
	
	SettingsManager::SettingsManager *settingsManager = SettingsManager::SettingsManager::instance();
	
	m_d->numTestRuns++;
	switch (m_d->numTestRuns) {
		case 1:
			Q_ASSERT(m_d->newsWebView);
			NetworkAccessManager::instance()->addRequestHandler("http://" + settingsManager->baseServer() + "/?module=DesktopBuzz", this, "handleBuzzRequest");
			tester->setTestPage(m_d->newsWebView->page());
			tester->scheduleTestFile(":/src/tests/BuzzTest.js");
			tester->runAllTests(Tester::Tester::DontWaitForLoad);
			break;
			
		case 2:
			NetworkAccessManager::instance()->removeRequestHandler("http://" + settingsManager->baseServer() + "/?module=DesktopBuzz");
			tester->setTestPage(m_d->webView->page());
			tester->scheduleTestFile(":/src/tests/PingTest.js");
			tester->scheduleTestFile(":/src/tests/LogoutTest.js");
			tester->scheduleTestFile(":/src/tests/LoginTest.js");
			tester->scheduleTestFile(":/src/tests/RosterTest.js");
			tester->scheduleTestFile(":/src/tests/LogoutTest.js");
			tester->runAllTests(Tester::Tester::DontWaitForLoad);
			break;
			
		case 3:
			// final tests are performed against the real chat server
			SystemSpy::SystemSpy::destroy();
			Jabber::Jabber::destroy();
			connect(Jabber::Jabber::instance(), SIGNAL(connected()), SLOT(loggedIn()));
			connect(Jabber::Jabber::instance(), SIGNAL(disconnected()), SLOT(loggedOut()));
			connect(Jabber::Jabber::instance(), SIGNAL(resourceConflict()), SLOT(loggedInFromOtherLocation()));
			SystemSpy::SystemSpy::instance();
			
			tester->scheduleTestFile(":/src/tests/LoginTest.js");
			tester->scheduleTestFile(":/src/tests/WwwDialogTest.js");
			tester->scheduleTestFile(":/src/tests/LogoutTest.js");
			tester->runAllTests(Tester::Tester::DontWaitForLoad);
			break;
			
		case 4:
			qApp->exit(m_d->totalNumFailedTests);
			break;
	}
}

NetworkReply *MainWindow::handleBuzzRequest(const QNetworkRequest &request) {
	
	QString path = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/news.html";
	
	return new NetworkReply(request, path, this);
}

} // namespace Kwekker
