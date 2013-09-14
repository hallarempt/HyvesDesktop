/****************************************************************************
**
** This file is part of a Qt Solutions component.
** 
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** 
** Contact:  Qt Software Information (qt-info@nokia.com)
** 
** Commercial Usage  
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
** 
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
** GNU General Public License Usage 
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** 
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
** 
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** 
****************************************************************************/

#ifndef QTIMAGEFILTER_H
#define QTIMAGEFILTER_H
#include <QtGui/QImage>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QStringList>
#include <QMutex>
#include <QMutexLocker>
#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QImage>
#include <QVariant>
#include "qtmatrix.h"
#include "imageselection.h"

#if defined(Q_WS_WIN)
#  if !defined(QT_QTIMAGEFILTERS_EXPORT) && !defined(QT_QTIMAGEFILTER_IMPORT)
#    define QT_QTIMAGEFILTERS_EXPORT
#  elif defined(QT_QTIMAGEFILTER_IMPORT)
#    if defined(QT_QTIMAGEFILTERS_EXPORT)
#      undef QT_QTIMAGEFILTERS_EXPORT
#    endif
#    define QT_QTIMAGEFILTERS_EXPORT __declspec(dllimport)
#  elif defined(QT_QTIMAGEFILTERS_EXPORT)
#    undef QT_QTIMAGEFILTERS_EXPORT
#    define QT_QTIMAGEFILTERS_EXPORT __declspec(dllexport)
#  endif
#else
#  define QT_QTIMAGEFILTERS_EXPORT
#endif

class ImageFilterThread;

class QT_QTIMAGEFILTERS_EXPORT QtImageFilter : public QObject {
	Q_OBJECT
public:
	enum FilterOption {
		FilterChannels = 1,
		FilterBorderPolicy,
		ConvolutionDivisor,
		ConvolutionBias,
		ConvolutionKernelMatrix,
		Radius,
		Center,
		Force,
		Deviation,
		Selection,
		InvertSelection,
		UserOption = 0x100
		     };
	
	QtImageFilter() {
		m_isAborting = false;
		m_currentSubFilter = 0;
		m_previewLabel = 0;
		m_totalProgress = 0;
		m_currentSubFilterTotalProgress = 0;
		m_progressTillLastSubFilter = 0;
		m_threadedPreviewEnabled = false;
		m_currentPreviewFilterThread = 0;
		m_isPreviewBeingClouded = false;
	}
	
	virtual QVariant option(int filteroption) const;
	
	virtual bool setOption(int filteroption, const QVariant &value);
	
	virtual bool supportsOption(int filteroption) const;
	
	virtual QImage apply(const QImage &img, const QRect& clipRect = QRect() ) = 0;
	
	virtual QString name() const = 0;
	
	virtual QString id() const = 0;
	
	virtual QString description() const;
	
	virtual ~QtImageFilter() {}
	
	virtual void abort() {
		if (m_currentSubFilter) {
			m_currentSubFilter->abort();
		}
		m_isAborting = true;
	}
	
	virtual void setNotAborting() {
		if (m_currentSubFilter) {
			m_currentSubFilter->setNotAborting();
		}
		m_isAborting = false;
	}

	void abortPreviewThread();

	virtual QWidget* controlsWidget(QLabel *previewLabel, QImage originalPreviewImage);
	
	/// sets the filter option values that's ideal to create a thumbnail image with
	virtual void setThumbnailCreationOptions(){
		resetOptions();
	};
	virtual bool hasSelection() const {
		return false;
	}
	virtual QList<int> options() const {
		return QList<int>();
	};
	virtual QString optionUsageString() const {
		QStringList ret;
		QList<int> supportedOptions = options();
		qSort(supportedOptions.begin(), supportedOptions.end());
		foreach(int i, supportedOptions) {
			if (supportsOption(i)) {
				QVariant opt = option(i);
				if (i == QtImageFilter::Selection) {
					ret << QString("%1: %2").arg(i).arg(hasSelection()? "present" : "absent");
				} else if (opt.canConvert(QVariant::Int)) {
					ret << QString("%1: %2").arg(i).arg(option(i).toInt());
				} else if (opt.canConvert(QVariant::String)) {
					ret << QString("%1: %2").arg(i).arg(option(i).toString());
				}
			}
		}
		if (ret.isEmpty()) {
			return "None";
		}
		return ret.join(", ");
	}
	
public slots:
	
	virtual void handleControlValueChanged();
	virtual void handlePreviewThreadFinished();
	
	/// resets the filter option values to the defaults;
	virtual void resetOptions(){};
	
signals:
	
	void progressChanged(int progress) const; // scale of 0 -100
	void addEllipseToSelection(); /// emitted when the filter wants an extra ellipse set on the selection
	void addRectToSelection();    /// emitted when the filter wants an extra rectangle set on the selection
	
protected:
	
	void setProgress(int progress);	
	
	/**
	 * create and return the specified filter.
	 *
	 * @param filter the ID string of the filter
	 * @param progressAmount the portion of the total progress this filter takes
	 * @reteurn the filter instance or 0 if the filter id was not correct
	 */
	QtImageFilter *setCurrentSubFilter(const QString& filter, int progressAmount = 100);
	
	/// delete the current filter and set to 0
	void unsetCurrentSubFilter();
	
	/// setThreadedPreviewEnabled: Makes the thread compute the preview in a thread
	/// Keeps the app responsive while previewing time-consuming effects
	/// If the preview change is caused by a slider that has tracking enabled, preview will not be threaded despite this setting
	/// This function is to be called in the derived filter's constructor
	void setThreadedPreviewEnabled(bool enabled);
	bool threadedPreviewEnabled() const;
	
	bool m_isAborting;
	QLabel *m_previewLabel;
	QImage m_originalPreviewImage;
	QMutex m_mutex;	
	
private slots:
	
	void setSubFilterProgress(int progress);
	void cloudPreview(); // make preview whitish, signifying preview is loading...
	
private:
	
	QtImageFilter* m_currentSubFilter;	
	int m_totalProgress;
	int m_progressTillLastSubFilter;
	int m_currentSubFilterTotalProgress;
	int m_currentSubFilterProgressAmount;
	ImageFilterThread *m_currentPreviewFilterThread;
	bool m_threadedPreviewEnabled;
	bool m_isPreviewBeingClouded;
};

class QT_QTIMAGEFILTERS_EXPORT QtImageFilterFactory {
public:
	typedef QtImageFilter* (*ImageFilterFactoryFunction)(void);
	static QtImageFilter *createImageFilter(const QString &id);
	
	static void registerImageFilter(const QString &id, ImageFilterFactoryFunction func);
	
	static QStringList imageFilterList();
	
#if !defined(QT_NO_MEMBER_TEMPLATES)
	template<typename T>
			static QtImageFilter *automaticFilterCreator()
	{
		return new T;
	}
	
	template<typename T>
			static void registerImageFilter(const QString &id)
	{
		registerImageFilter(id, &automaticFilterCreator<T>);
	}
#endif
	
};


// For compilers with broken template member function support and for those that want to keep
// portability high.
template<typename T>
		QtImageFilter *qtAutomaticFilterCreator(const T * /*dummy*/ = 0)  { return new T;}

template<typename T>
		void qtRegisterImageFilter(const QString &id, T * /*dummy*/ = 0)
{
	typedef QtImageFilter* (* ConstructPtr)(const T*);
	ConstructPtr cptr = qtAutomaticFilterCreator<T>;
	QtImageFilterFactory::registerImageFilter(id,
						  reinterpret_cast<QtImageFilterFactory::ImageFilterFactoryFunction>(cptr));
}

typedef QtMatrix<int> QtConvolutionKernelMatrix;
Q_DECLARE_METATYPE(QtConvolutionKernelMatrix)
		
		
#endif // QTIMAGEFILTER_H
		
		
