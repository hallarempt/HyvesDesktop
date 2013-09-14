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

#include <QApplication>
#include <QDesktopWidget>

#include "extender/Extender.h"
#include "ContactToolTip.h"
#include "ToolTipper.h"
#include "Util.h"

namespace ToolTipper {

ToolTipper::ToolTipper(QWidget *referenceWidget, QObject *parent) :
	QObject(parent),
	m_referenceWidget(referenceWidget),
	m_toolTip(0) {
	
	Q_ASSERT(m_referenceWidget != 0);
	
	Extender::Extender::instance()->registerObject("toolTipper", this);
}

ToolTipper::~ToolTipper() {
	
	hideToolTip(ForceHide);
}

void ToolTipper::showContactToolTip(const QString &parametersJson) {
	
	hideToolTip(ForceHide);
	
	QVariantMap parameters = Util::jsonToVariantMap(parametersJson);
	
	m_toolTip = new ContactToolTip(parameters);
	
	positionToolTip(m_toolTip,
		parameters["left"].toInt(), parameters["top"].toInt(), parameters["right"].toInt(), parameters["width"].toInt()
	);
	
	connect(m_toolTip, SIGNAL(destroyed()), SLOT(toolTipDestroyed()));
}

void ToolTipper::hideToolTip(ToolTipper::Options options) {
	
	if (m_toolTip && (options & ForceHide || !m_toolTip->isHovered())) {
		m_toolTip->disconnect();
		m_toolTip->deleteLater();
		m_toolTip = 0;
	}
}

void ToolTipper::positionToolTip(QWidget *toolTip, int parentLeft, int parentTop, int parentRight, int width) {
	
	QDesktopWidget *desktop = QApplication::desktop();
	QRect availableGeometry = desktop->availableGeometry(m_referenceWidget);
	
	QPoint referencePosition = m_referenceWidget->mapToGlobal(QPoint(0, 0));
	parentLeft += referencePosition.x();
	parentTop += referencePosition.y();
	parentRight += referencePosition.x();
	
	int left = 0;
	int top = 0;
	if (parentLeft - m_toolTip->width() < 0) {
		left = parentRight;
		if (left + width > availableGeometry.width()) {
			left = availableGeometry.width() - width;
		}
	} else {
		left = parentLeft - toolTip->width();
	}
	if (parentTop + m_toolTip->height() > availableGeometry.height()) {
		top = availableGeometry.height() - m_toolTip->height();
	} else {
		top = parentTop;
	}
	
	toolTip->move(left, top);
}

void ToolTipper::toolTipDestroyed() {
	
	m_toolTip = 0;
}

} // namespace ToolTipper
