#ifndef ZIPCACHE_H
#define ZIPCACHE_H

#include <QMap>
#include <QObject>

class QuaZip;

namespace Zipper {

class ZipCache : public QObject {
	
	Q_OBJECT
	
	public:
		static ZipCache *instance();
		static void destroy();
		
		QuaZip *openZip(const QString &zipFileName);
		
	private:
		static ZipCache *s_instance;
		
		QMap<QString, QuaZip *> m_map;
		
		ZipCache();
		virtual ~ZipCache();
		
	private slots:
		void cleanup();
};

} // namespace Zipper

#endif // ZIPCACHE_H
