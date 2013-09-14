#include <QtDebug>
#include <QCoreApplication>

#include "quazip.h"

#include "ZipCache.h"

namespace Zipper {

ZipCache *ZipCache::s_instance = 0;

ZipCache *ZipCache::instance() {
	
	if (s_instance == 0) {
		s_instance = new ZipCache();
	}
	
	return s_instance;
}

void ZipCache::destroy() {
	
	delete s_instance;
	s_instance = 0;
}
	
QuaZip *ZipCache::openZip(const QString &zipFileName) {
	
	if (!m_map.contains(zipFileName)) {
		QuaZip *zip = new QuaZip(zipFileName);
		if (!zip->open(QuaZip::mdUnzip)) {
			qWarning() << "ZipCache::openZip(): zip.open(): " << zip->getZipError();
			return 0;
		}
		
		zip->buildLookupIndex();
		m_map[zipFileName] = zip;
	}
	
	return m_map[zipFileName];
}

ZipCache::ZipCache() {
	
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
}

ZipCache::~ZipCache() {
	
	foreach (QuaZip *zip, m_map) {
		delete zip;
	}
}

void ZipCache::cleanup() {
	
	destroy();
}

} // namespace Zipper
