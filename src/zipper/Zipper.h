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

#ifndef ZIPPER_H
#define ZIPPER_H

#include <QByteArray>
#include <QIODevice>
#include <QString>

#include <hyveslib_export.h>

namespace Zipper {

/**
 * Zips the entire contents of a directory.
 *
 * @param dirPath Path to the directory to zip.
 * @param zipPath Path to the resulting zip archive.
 * @return @c true on success, @c false on error.
 */
bool HYVESLIB_EXPORT zipDirectory(const QString &dirPath, const QString &zipPath);

/**
 * Unzips a zip archive to a directory.
 *
 * @param zipPath Path to the zip archive to unzip.
 * @param destinationPath Path to the destination directory.
 * @return @c true on success, @c false on error.
 */
bool HYVESLIB_EXPORT unzipArchive(const QString &zipPath, const QString &destinationPath);

/**
 * Reads all content from a file contained in a zip archive.
 *
 * @param zipPath Path to the zip archive to read from.
 * @param filePath Internal file path of the exact file to read. Should begin
 *                 with a slash (/).
 * @return A byte array containing the contents of the file. In case of an
 *         error, the array will be empty.
 */
QByteArray HYVESLIB_EXPORT readFileFromArchive(const QString &zipPath, const QString &filePath);

/**
 * Extracts and copies a file from a zip archive to a location on disk.
 *
 * @param zipPath Path to the zip archive to read from.
 * @param filePath Internal file path of the exact file to read. Should begin
 *                 with a slash (/).
 * @param destinationPath Path where the file should be written on disk.
 * @return @c true on success, @c false on error.
 */
bool HYVESLIB_EXPORT copyFileFromArchive(const QString &zipPath, const QString &filePath, const QString &destinationPath);

/**
 * A read-only QIODevice representing a single file inside a Zip archive.
 */
class HYVESLIB_EXPORT ZippedFile : public QIODevice {
	
	Q_OBJECT
	
	public:
		ZippedFile(const QString &zipFileName, const QString &fileName);
		virtual ~ZippedFile();
		
		virtual bool open(OpenMode mode);
		
		virtual qint64 bytesAvailable() const;
		
		static bool exists(const QString &zipFileName, const QString &fileName);
		
	protected:
		virtual qint64 readData(char *data, qint64 maxLength);
		virtual qint64 writeData(const char *, qint64);
		
	private:
		struct Private;
		Private *const m_d;
};

} // namespace Zipper

#endif // ZIPPER_H
