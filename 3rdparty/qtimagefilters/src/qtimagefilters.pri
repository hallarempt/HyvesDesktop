include(../common.pri)
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
qtimagefilters-uselib:!qtimagefilters-buildlib:LIBS += -L$$QTIMAGEFILTERS_LIBDIR \
    -l$$QTIMAGEFILTERS_LIBNAME
else { 
    SOURCES += convolutionfilter.cpp \
        qtimagefilter.cpp \
        punchfilter.cpp \
	blurfilter.cpp \
        qtimagefilterfactory.cpp
    HEADERS += qtimagefilter.h \
        convolutionfilter.h \
        gaussfilter.h \
        blurfilter.h \
        punchfilter.h \
        qtmatrix.h
    
    # Additional Hyves filters
    SOURCES += charcoalfilter.cpp \
	imageselection.cpp \
	imagefilterthread.cpp \
        colorizefilter.cpp \
	colorwell.cpp \
        grayfilter.cpp \
	gaussfilter.cpp \
        invertfilter.cpp \
        normalizefilter.cpp \
        sepiafilter.cpp \
        oilpaintfilter.cpp \
        cinemafilter.cpp \
	posterizefilter.cpp \
	vintagefilter.cpp \
	filmgrainfilter.cpp \
	removecolorfilter.cpp \
	thresholdfilter.cpp \
	warholfilter.cpp \
	highpassfilter.cpp \
	grittyfilter.cpp\
	desaturatefilter.cpp\
	curvesfilter.cpp \
	noise.cpp \
	noisefilter.cpp \
	ditherfilter.cpp \
	unsharpmaskfilter.cpp \
	photodither.cpp\
	autocontrastfilter.cpp \
	pixelatefilter.cpp \
	frostedglassfilter.cpp \
        texturefilter.cpp \
	defocusfilter.cpp
    HEADERS += charcoalfilter.h \
        colorizefilter.h \
	imageselection.h \
	imagefilterthread.h \
	pixelatefilter.h \
        grayfilter.h \
	unsharpmaskfilter.h \
	colorwell.h \
	ditherfilter.h \
        invertfilter.h \
        normalizefilter.h \
        sepiafilter.h \
        oilpaintfilter.h \
	frostedglassfilter.h \
	posterizefilter.h \
	vintagefilter.h \
	colorconversions.h \
	photodither.h\
	filmgrainfilter.h \
	removecolorfilter.h \
	thresholdfilter.h \
	warholfilter.h \
	blendingmodes.h\
	highpassfilter.h\
	grittyfilter.h\
	desaturatefilter.h\
	curvesfilter.h \
	noise.h \
	noisefilter.h \
	autocontrastfilter.h \
        cinemafilter.h \
        texturefilter.h \
	defocusfilter.h
RESOURCES += qtimagefilters.qrc
}
win32 { 
    contains(TEMPLATE, lib):contains(CONFIG, shared):DEFINES += QT_QTIMAGEFILTERS_EXPORT
    else:qtimagefilters-uselib:DEFINES += QT_QTIMAGEFILTERS_IMPORT
}
