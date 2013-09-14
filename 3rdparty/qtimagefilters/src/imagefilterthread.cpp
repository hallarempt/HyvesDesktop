#include "imagefilterthread.h"
#include "imageselection.h"
#include <QDebug>
#include <QMessageBox>

ImageFilterThread::ImageFilterThread(QObject *parent)
		: QThread(parent) {
	m_imageFilter = 0;
	m_isAborted = false;
}

void ImageFilterThread::setImageFilter(QtImageFilter *filter) {
	m_imageFilter = filter;
	connect(m_imageFilter, SIGNAL(progressChanged(int)), this, SIGNAL(progressChanged(int)));
}

QtImageFilter *ImageFilterThread::imageFilter() const {
	return m_imageFilter;
}

void ImageFilterThread::clearImageFilter() {
	if (m_imageFilter) {
		disconnect(m_imageFilter, SIGNAL(progressChanged(int)), this, SIGNAL(progressChanged(int)));
	}
	m_imageFilter = 0;
}

void ImageFilterThread::setImage(const QImage &image) {
	m_image = image;
	m_clipRect = QRect();
}

QImage ImageFilterThread::image() const {
	return m_image;
}

void ImageFilterThread::setClipRect(const QRect &rect) {
	m_clipRect = rect;
}

QRect ImageFilterThread::clipRect() const {
	return m_clipRect;
}

void ImageFilterThread::run() {
	// Should ideally use mutex here, but not doing so. Assumption is that the variables
	// will not be changed while the filter is being applied
	m_isAborted = false;
	m_timer.start();
	m_resultImage = m_imageFilter->apply(m_image);
}

QImage ImageFilterThread::resultImage() const {
	return m_resultImage;
}

void ImageFilterThread::abort() {
	m_isAborted = true;
	m_imageFilter->abort();
	m_resultImage = QImage();
}

bool ImageFilterThread::isAborted() const {
	return m_isAborted;
}

int ImageFilterThread::elapsed() const {
	return m_timer.elapsed();
}
