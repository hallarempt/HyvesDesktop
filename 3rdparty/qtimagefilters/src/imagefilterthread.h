#ifndef IMAGEFILTERTHREAD_H
#define IMAGEFILTERTHREAD_H

#include <QThread>
#include <QImage>
#include <QList>
#include <QGraphicsItem>
#include <QTime>

#include <QtImageFilter>

class ImageFilterThread : public QThread
{
	Q_OBJECT
public:
	ImageFilterThread(QObject *parent = 0);
	
	void setImageFilter(QtImageFilter *filter);
	QtImageFilter *imageFilter() const;
	void clearImageFilter();
	
	void setImage(const QImage &image);
	QImage image() const;
	
	void setClipRect(const QRect &rect);
	QRect clipRect() const;
	
	void run();
	int elapsed() const;
	
	QImage resultImage() const;
	
	void abort();
	bool isAborted() const;

signals:
	void progressChanged(int progress) const;

private:
	QImage m_image;
	QImage m_resultImage;
	QRect m_clipRect;
	QtImageFilter *m_imageFilter;
	QList<QGraphicsItem*> m_selection;
	bool m_isAborted;
	QTime m_timer;
};

#endif // IMAGEFILTERTHREAD_H
