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
#include "ImageSelectionTest.h"

#include <QtTest>
#include <QPainter>
#include <QLabel>
#include <QPainterPath>
#include <QMessageBox>
#include "imageselection.h"

void ImageSelectionTest::testCreation() {
	
	ImageSelection selection(QSize(500,500));
	QVERIFY(selection.isGrayscale());
	QVERIFY(!selection.isNull());
	for (int row = 0; row < 500; ++row) {
		QRgb* line = reinterpret_cast<QRgb*>(selection.scanLine(row));
		for (int col = 0; col < 500; ++col) {
			QVERIFY(line[col] == qRgb(0, 0, 0));
		}
	}
	for (int row = 0; row < 500; ++row) {
		for (int col = 0; col < 500; ++col) {
			QVERIFY(QColor(selection.pixel(col, row)) == Qt::black);
		}
	}
	QVERIFY(selection.selectionThreshold() == 128);
	
}

void ImageSelectionTest::testClear() {
	
	ImageSelection selection(QSize(500,500));
	QPainter gc(&selection);
	gc.fillRect(0, 0, 250, 250, Qt::white);
	gc.end();
	selection.clear();
	for (int row = 0; row < 500; ++row) {
		QRgb* line = reinterpret_cast<QRgb*>(selection.scanLine(row));
		for (int col = 0; col < 500; ++col) {
			QVERIFY(line[col] == qRgb(0,0,0));
		}
	}
	
}

void ImageSelectionTest::testAdd() {
	
	ImageSelection selection(QSize(500, 500));
	QPainterPath path;
	path.addEllipse(selection.rect());
	selection.addPainterPath(path);
	QImage compare(QString(FILES_DATA_DIR) + QDir::separator() + "add.png");
	if (selection != compare) {
		selection.save("test_image_selection_add.png");
		QFAIL("adding selection failed");
	}
	

	
}


void ImageSelectionTest::testSubtract() {
	
	ImageSelection selection(QSize(500, 500));
	{
		QPainterPath path;
		path.addEllipse(selection.rect());
		selection.addPainterPath(path);
	}
	{	
		QPainterPath path;
		path.addEllipse(selection.rect());
		selection.subtractPainterPath(path);
	}
	QImage compare(QString(FILES_DATA_DIR) + QDir::separator() + "subtract.png");
	if (selection != compare) {
		selection.save("test_image_selection_subtract.png");
		QFAIL("subtracting selection failed");
	}

}

QTEST_MAIN(ImageSelectionTest);
