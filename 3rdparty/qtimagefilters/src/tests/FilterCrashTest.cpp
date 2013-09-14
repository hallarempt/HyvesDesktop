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

#include <QtTest>

#include "FilterCrashTest.h"
#include "qtimagefilter.h"

QImage FilterCrashTest::createTestImage() {

	QImage image( 4096,  4096,  QImage::Format_RGB32 );
	int x, y;
	x = y = 0;
	for (int r = 0; r < 256; ++r) {
		for (int g = 0; g < 256; ++g) {
			for (int b = 0; b < 256; ++b) {
				if (y == 4096) {
					break;
				}
				QRgb rgb = qRgb(r, g, b);
				image.setPixel(x, y, rgb);
				if (x == 4095) {
					x = 0;
					y ++;
				}
				else {
					x++;
				}
			}
		}
	}
	
	image.save("allcolors.png");
	return image;
}

void FilterCrashTest::testAllFilters() {

	QImage image = createTestImage();
	
	QStringList filters = QtImageFilterFactory::imageFilterList();
	
	foreach(const QString& filterName, filters) {
	
		qDebug() << "testing filter " << filterName;
		QtImageFilter* filter = QtImageFilterFactory::createImageFilter(filterName);
		QImage result = filter->apply(image, image.rect());
		result.save("test_result_" + filterName + ".png");
		delete filter;
	}
}


QTEST_MAIN(FilterCrashTest);
