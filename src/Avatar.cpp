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

#include <QDesktopServices>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QUrl>

#include "mediacache/MediaCache.h"
#include "Avatar.h"

Avatar::Avatar(const QString &mediaId, const QString &url, QWidget *parent) :
	QWidget(parent),
	m_mediaId(mediaId),
	m_url(url) {
	
	setFixedSize(79, 82);
	
	if (!url.isEmpty()) {
		setCursor(Qt::PointingHandCursor);
	}
}

Avatar::~Avatar() {
}

void Avatar::mouseReleaseEvent(QMouseEvent *event) {
	
	if (event->button() == Qt::LeftButton) {
		QDesktopServices::openUrl(m_url);
	}
}

void Avatar::paintEvent(QPaintEvent *event) {
	
	QPainter painter(this);
	
	QImage backgroundImage(":/data/avatar-bg-75.png", "PNG");
	painter.drawImage(0, 0, backgroundImage);
	
	QImage avatarImage = MediaCache::MediaCache::instance()->image(m_mediaId, QSize(75, 75));
	painter.drawImage(2, 2, avatarImage);
}
