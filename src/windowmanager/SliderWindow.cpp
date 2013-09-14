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

#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QTimeLine>
#include <QTimer>
#include <QWebFrame>

#include "extender/Extender.h"
#include "logger/Logger.h"
#include "SliderWindow.h"
#include "WebView.h"

#ifdef Q_WS_WIN
#include <Windows.h>

static bool isFullScreenAppRunning() {
	
	HWND hWnd = GetForegroundWindow();
	
	RECT rc;
	GetWindowRect(hWnd, &rc);
	
	return (rc.right - rc.left >= GetSystemMetrics(SM_CXSCREEN) && 
	        rc.bottom - rc.top >= GetSystemMetrics(SM_CYSCREEN));
}
#endif

namespace WindowManager {

static const int sliderWidth = 200;
static const int sliderHeight = 115;

static const int sliderRightOffset = 30;

static const int slideDuration = 500;
static const int holdDuration = 6500;

struct SliderWindow::Private {
	Logger::Logger *log;
	
	WebView *webView;
	
	int index;
	
	int left;
	int bottom;
	
	QTimeLine timeLine;
	QTimer timer;
	
	bool isAnimating;
	bool isHovered;
	
	Private() :
		log(0),
		webView(0),
		index(0),
		left(0),
		bottom(0),
		isAnimating(false),
		isHovered(false) {
	}
	
	~Private() {
		
		delete log;
	}
};

SliderWindow::SliderWindow() :
	QWidget(),
	m_d(new Private()) {
	
	m_d->log = new Logger::Logger("SliderWindow");
	
	m_d->webView = new WebView(this);
	// Hide the inspect/reload/stop etc. context menu
	m_d->webView->setContextMenuPolicy(Qt::NoContextMenu);
	
	Extender::Extender::instance()->registerEnvironment(m_d->webView->page()->mainFrame());
	
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_d->webView);
	
	m_d->timeLine.setDuration(slideDuration);
	
	setWidgetProperties();
	
	connect(&m_d->timeLine, SIGNAL(frameChanged(int)), SLOT(animate(int)));
	connect(&m_d->timeLine, SIGNAL(finished()), SLOT(timeLineFinished()));
	connect(&m_d->timer, SIGNAL(timeout()), SLOT(startReverseAnimation()));
}

SliderWindow::~SliderWindow() {
	
	delete m_d;
}

void SliderWindow::setIndex(int index) {
	
	m_d->index = index;
}

void SliderWindow::setContent(const QString &htmlContent) {
	
	QString content(htmlContent);
	m_d->webView->setHtml(content.replace("#windowId#", QString::number(m_d->index)));
}

void SliderWindow::startAnimation() {
	
#ifdef Q_WS_WIN
	if (isFullScreenAppRunning()) {
		return;
	}
#endif
	
	initializePosition();
	updateSizeAndPosition(0);
	show();
#ifdef Q_WS_X11
	raise();
#endif
	m_d->timeLine.setFrameRange(0, sliderHeight);
	m_d->timeLine.setDirection(QTimeLine::Forward);
	m_d->timeLine.start();
	
	m_d->isAnimating = true;
	m_d->isHovered = false;
}

void SliderWindow::stopAnimation() {
	
	m_d->timeLine.stop();
	m_d->timer.stop();
	
	hide();
	
	m_d->isAnimating = false;
}

bool SliderWindow::isAnimating() const {
	
	return m_d->isAnimating;
}

void SliderWindow::enterEvent(QEvent *event) {
	
	m_d->isHovered = true;
}

void SliderWindow::leaveEvent(QEvent *event) {
	
	m_d->isHovered = false;
	
	if (m_d->timeLine.direction() == QTimeLine::Backward) {
		m_d->timer.start(1000);
	}
}

void SliderWindow::setWidgetProperties() {
	
#ifdef Q_WS_WIN
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
#else
#ifdef Q_WS_MAC
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
#else
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
#endif
#endif
	setAttribute(Qt::WA_ShowWithoutActivating, true);
	setFocusPolicy(Qt::NoFocus);
}

void SliderWindow::initializePosition() {
	
	QDesktopWidget desktopWidget;
	QRect availableGeometry = desktopWidget.availableGeometry(desktopWidget.primaryScreen());
	
	m_d->left = availableGeometry.width() - sliderRightOffset - sliderWidth;
	m_d->bottom = availableGeometry.height() - m_d->index * sliderHeight;
}

void SliderWindow::updateSizeAndPosition(int frame) {
	
	move(m_d->left, m_d->bottom - frame);
	setFixedSize(sliderWidth, frame);
}

void SliderWindow::animate(int frame) {
	
	updateSizeAndPosition(frame);
}

void SliderWindow::timeLineFinished() {
	
	if (m_d->timeLine.direction() == QTimeLine::Forward) {
		m_d->timer.start(holdDuration);
	} else {
		hide();
		emit animationFinished();
		m_d->isAnimating = false;
	}
}

void SliderWindow::startReverseAnimation() {
	
	m_d->timer.stop();
	
	m_d->timeLine.setDirection(QTimeLine::Backward);
	
	if (!m_d->isHovered) {
		m_d->timeLine.start();
	}
}

} // namespace WindowManager
