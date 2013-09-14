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

#ifndef SLIDERWINDOW_H
#define SLIDERWINDOW_H

#include <QWidget>

namespace WindowManager {

/**
 * Displays a slider window which moves up and down in the bottom right corner
 * of the screen.
 */
class SliderWindow : public QWidget {
	
	Q_OBJECT
	
	public:
		SliderWindow();
		virtual ~SliderWindow();
		
		/**
		 * Sets the index of the slider. The index determines the actual
		 * position where the slider is placed. The first slider (index 0) is
		 * placed right above the task bar, the second slider (index 1) right
		 * above the first slider, and so on.
		 * 
		 * @param index The slider index.
		 */
		void setIndex(int index);
		
		/**
		 * Sets the HTML content to be displayed in the slider.
		 * 
		 * @param htmlContent HTML content to be displayed.
		 */
		void setContent(const QString &htmlContent);
		
		/**
		 * Starts the animation.
		 */
		void startAnimation();
		
		/**
		 * Stops the animation and hides the slider.
		 */
		void stopAnimation();
		
		/**
		 * Returns whether the slider is currently animating.
		 * 
		 * @return @c true if the slider is animating, @c false otherwise.
		 */
		bool isAnimating() const;
		
	signals:
		/**
		 * Emitted when the animation is finished. After this, the slider can be
		 * re-used by setting new content and re-starting the animation.
		 */
		void animationFinished();
		
	protected slots:
		void enterEvent(QEvent *event);
		void leaveEvent(QEvent *event);
		
	private:
		struct Private;
		Private *const m_d;
		
		void setWidgetProperties();
		
		void initializePosition();
		void updateSizeAndPosition(int frame);
		
	private slots:
		void animate(int frame);
		
		void timeLineFinished();
		
		void startReverseAnimation();
};

} // namespace WindowManager

#endif // SLIDERWINDOW_H
