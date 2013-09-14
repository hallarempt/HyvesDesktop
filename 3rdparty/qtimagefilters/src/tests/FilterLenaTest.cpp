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
#include "FilterLenaTest.h"
#include "qtimagefilter.h"


void FilterLenaTest::testAllFilters() {


	QString filename = QString(FILES_DATA_DIR).replace("/", QDir::separator()) + QDir::separator() + "lena_std.tif";
	QImage image(filename);
	Q_ASSERT(!image.isNull());
	QStringList filters = QtImageFilterFactory::imageFilterList();
	
	QStringList failures;
	foreach(const QString& filterName, filters) {
	
		QtImageFilter* filter = QtImageFilterFactory::createImageFilter(filterName);
		QImage result = filter->apply(image, image.rect());
		
		QImage compare(QString(FILES_DATA_DIR).replace("/", QDir::separator()) + QDir::separator() + "test_result_lena" + filterName + ".png");
		if (result != compare) {
			result.save("test_lena_failure_" + filterName + ".png");
			failures << filterName;
		}
		
		delete filter;
	}
	
}

void FilterLenaTest::testEmptyImageFilters() {

	QImage image;
	QStringList filters = QtImageFilterFactory::imageFilterList();
	foreach(const QString& filterName, filters) {
	
		//qDebug() << "testing filter on empty image" << filterName; 
		QtImageFilter* filter = QtImageFilterFactory::createImageFilter(filterName);
		QImage result = filter->apply(image, image.rect());
		delete filter;
	}
}


QTEST_MAIN(FilterLenaTest);
