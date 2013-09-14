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

#ifndef TOOLTIPPER_H
#define TOOLTIPPER_H

#include <QObject>
#include "hyveslib_export.h"
class QWidget;

namespace ToolTipper {

class ContactToolTip;

class HYVESLIB_EXPORT ToolTipper : public QObject {
	
	Q_OBJECT
	
	public:
		enum Options {
			NoOptions = 0,
			ForceHide = 1
		};
		
		ToolTipper(QWidget *referenceWidget, QObject *parent = 0);
		virtual ~ToolTipper();
		
	public slots:
		void showContactToolTip(const QString &parametersJson);
		void hideToolTip(Options options = NoOptions);
		
	private:
		QWidget *m_referenceWidget;
		ContactToolTip *m_toolTip;
		
		void positionToolTip(QWidget *toolTip, int parentLeft, int parentTop, int parentRight, int width);
		
	private slots:
		void toolTipDestroyed();
};

} // namespace ToolTipper

#endif // TOOLTIPPER_H
