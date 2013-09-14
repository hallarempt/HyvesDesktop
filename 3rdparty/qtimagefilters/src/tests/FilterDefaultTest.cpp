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

#include "FilterDefaultTest.h"
#include "qtimagefilter.h"
#include "grittyfilter.h"

void FilterDefaultTest::testGritty() {

	QtImageFilter* gritty = QtImageFilterFactory::createImageFilter("Gritty");
	
	int contrast = gritty->option(GrittyFilter::Contrast).toInt();
	QVERIFY(gritty->setOption(GrittyFilter::Contrast, contrast * 2));
	QCOMPARE(contrast * 2, gritty->option(GrittyFilter::Contrast).toInt());
	
	gritty->resetOptions();
	QCOMPARE(contrast, gritty->option(GrittyFilter::Contrast).toInt());
	
	delete gritty;

}


QTEST_MAIN(FilterDefaultTest);
