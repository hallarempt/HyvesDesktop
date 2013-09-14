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

#include <QtDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>

#include "quazip.h"
#include "quazipfile.h"

#include "Zipper.h"
#include "ZipCache.h"

namespace Zipper {

bool zipDirectory(const QString &dirPath, const QString &zipPath) {
	
	QuaZip zip(zipPath);
	if (!zip.open(QuaZip::mdCreate)) {
		qWarning() << "zipDirectory(): zip.open(): " << zip.getZipError();
		return false;
	}
	
	QFile inFile;
	QuaZipFile outFile(&zip);
	QDirIterator it(dirPath, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		QString filePath = it.next();
		if (!it.fileInfo().isFile()) {
			continue;
		}
		
		inFile.setFileName(filePath);
		if (!inFile.open(QIODevice::ReadOnly)) {
			qWarning() << "zipDirectory(): inFile.open(): " << inFile.errorString();
			return false;
		}
		
		QString relativePath = filePath.mid(dirPath.length());
		if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(relativePath, inFile.fileName()))) {
			qWarning() << "zipDirectory(): outFile.open(): " << outFile.getZipError();
			return false;
		}
		
		QByteArray content = inFile.readAll();
		outFile.write(content);
		
		if (outFile.getZipError() != UNZ_OK) {
			qWarning() << "zipDirectory(): outFile.write(): " << outFile.getZipError();
			return false;
	    }
		
		outFile.close();
		if (outFile.getZipError() != UNZ_OK) {
			qWarning() << "zipDirectory(): outFile.close(): " << outFile.getZipError();
			return false;
		}
		inFile.close();
	}
	
	return true;
}

bool unzipArchive(const QString &zipPath, const QString &destinationPath) {
	
	QuaZip zip(zipPath);
	if (!zip.open(QuaZip::mdUnzip)) {
		qWarning() << "unzipArchive(): zip.open(): " << zip.getZipError();
		return false;
	}
	
	QFile outFile;
	QuaZipFile inFile(&zip);
	QuaZipFileInfo info;
	for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
		if (!zip.getCurrentFileInfo(&info)) {
			qWarning() << "unzipArchive(): getCurrentFileInfo(): " << zip.getZipError();
			return false;
		}
		
		if (!inFile.open(QIODevice::ReadOnly)) {
			qWarning() << "unzipArchive(): inFile.open(): " << inFile.getZipError();
			return false;
		}
		
		QString outFilename = destinationPath + inFile.getActualFileName();
		if (inFile.getZipError() != UNZ_OK) {
			qWarning() << "unzipArchive(): inFile.getFileName(): " << inFile.getZipError();
			return false;
		}
		
		QDir().mkpath(outFilename.left(outFilename.lastIndexOf("/")));
		
		outFile.setFileName(outFilename);
		if (!outFile.open(QIODevice::WriteOnly)) {
			qWarning() << "unzipArchive(): outFile.open(): " << outFile.errorString();
		}
		
		QByteArray content = inFile.readAll();
		if (!outFile.write(content)) {
			qWarning() << "unzipArchive(): outFile.write(): " << outFile.errorString();
			return false;
		}
		
		outFile.close();
		inFile.close();
	}
	
	return true;
}

QByteArray readFileFromArchive(const QString &zipPath, const QString &filePath) {
	
	QByteArray content;
	
	QuaZip zip(zipPath);
	if (!zip.open(QuaZip::mdUnzip)) {
		qWarning() << "readFileFromArchive(): zip.open(): " << zip.getZipError();
		return content;
	}
	
	for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
		if (zip.getCurrentFileName() != filePath) {
			continue;
		}
		
		QuaZipFile inFile(&zip);
		if (!inFile.open(QIODevice::ReadOnly)) {
			qWarning() << "readFileFromArchive(): inFile.open(): " << inFile.getZipError();
			break;
		}
		
		content = inFile.readAll();
		inFile.close();
		break;
	}
	
	return content;
}

bool copyFileFromArchive(const QString &zipPath, const QString &filePath, const QString &destinationPath) {
	
	ZipCache *zipCache = ZipCache::instance();
	QuaZip *zip = zipCache->openZip(zipPath);
	if (!zip) {
		return false;
	}
	
	QString fileName = (filePath.startsWith("/") ? filePath.mid(1) : filePath);
	if (!zip->setIndexedCurrentFile(fileName)) {
		qWarning() << "copyFileFromArchive(): Could not find file in zip file: " << fileName;
		return false;
	}
	
	QuaZipFile inFile(zip);
	if (!inFile.open(QIODevice::ReadOnly)) {
		qWarning() << "copyFileFromArchive(): inFile.open(): " << inFile.getZipError();
		return false;
	}
	
	QByteArray contents = inFile.readAll();
	inFile.close();
	
	QFile destinationFile(destinationPath);
	bool opened = destinationFile.open(QIODevice::WriteOnly);
	if (!opened) {
		qWarning() << "copyFileFromArchive(): Could not open destination file for writing: " << destinationPath;
		return false;
	}
	
	qint64 bytesWritten = destinationFile.write(contents);
	destinationFile.close();
	if (bytesWritten < contents.length()) {
		qWarning() << "copyFileFromArchive(): Could not write (all) data to destination file: " << destinationPath;
		return false;
	}
	
	return true;
}

struct ZippedFile::Private {
	QByteArray data;
	bool open;
};

ZippedFile::ZippedFile(const QString &zipFileName, const QString &fileName) :
	m_d(new Private()) {
	
	m_d->open = false;
	
	ZipCache *zipCache = ZipCache::instance();
	QuaZip *zip = zipCache->openZip(zipFileName);
	if (!zip) {
		return;
	}
	
	QString _fileName = (fileName.startsWith("/") ? fileName.mid(1) : fileName);
	if (!zip->setIndexedCurrentFile(_fileName)) {
		qWarning() << "ZippedFile::ZippedFile(): Could not find file in zip file: " << fileName;
		return;
	}
	
	QuaZipFile inFile(zip);
	if (!inFile.open(QIODevice::ReadOnly)) {
		qWarning() << "ZippedFile::ZippedFile(): inFile.open(): " << inFile.getZipError();
		return;
	}
	
	m_d->data = inFile.readAll();
	m_d->open = true;
	inFile.close();
}

ZippedFile::~ZippedFile() {
	
	delete m_d;
}

bool ZippedFile::open(OpenMode mode) {
	
	if (mode != QIODevice::ReadOnly) {
		return false;
	}
	if (!m_d->open) {
		return false;
	}
	
	setOpenMode(mode);
	return true;
}

qint64 ZippedFile::bytesAvailable() const {
	
	return m_d->data.length() + QIODevice::bytesAvailable();
}

bool ZippedFile::exists(const QString &zipFileName, const QString &fileName) {
	
	ZipCache *zipCache = ZipCache::instance();
	QuaZip *zip = zipCache->openZip(zipFileName);
	if (!zip) {
		return false;
	}
	
	QString _fileName = (fileName.startsWith("/") ? fileName.mid(1) : fileName);
	return zip->indexContains(_fileName);
}

qint64 ZippedFile::readData(char *buffer, qint64 maxLength) {
	
	qint64 length = qMin(qint64(m_d->data.length()), maxLength);
	if (length) {
		qMemCopy(buffer, m_d->data.constData(), length);
		m_d->data.remove(0, length);
	}
	return length;
}

qint64 ZippedFile::writeData(const char *, qint64) {
	
	return 0; // writing to zip archives is not supported
}

} // namespace Zipper
