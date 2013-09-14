add_subdirectory(tests)

SET(qtimagefilters_SOURCES
	charcoalfilter.cpp
	imageselection.cpp
	imagefilterthread.cpp
	colorizefilter.cpp
	blurfilter.cpp
	colorwell.cpp
	convolutionfilter.cpp
	grayfilter.cpp
	invertfilter.cpp
	normalizefilter.cpp
	punchfilter.cpp
	sepiafilter.cpp
	cinemafilter.cpp
	oilpaintfilter.cpp
	gaussfilter.cpp
	photodither.cpp
	frostedglassfilter.cpp
	posterizefilter.cpp
	vintagefilter.cpp
	filmgrainfilter.cpp
	desaturatefilter.cpp
	qtimagefilter.cpp
	noise.cpp
	removecolorfilter.cpp
	thresholdfilter.cpp
	warholfilter.cpp
	highpassfilter.cpp
	grittyfilter.cpp
	curvesfilter.cpp
	noisefilter.cpp
	ditherfilter.cpp
	unsharpmaskfilter.cpp
	autocontrastfilter.cpp
	texturefilter.cpp
	pixelatefilter.cpp
	defocusfilter.cpp
	qtimagefilterfactory.cpp
)

SET(qtimagefilters_HEADERS
	charcoalfilter.h
	blurfilter.h
	imageselection.h
	imagefilterthread.h
	unsharpmaskfilter.h
	colorizefilter.h
	colorwell.h
	ditherfilter.h
	pixelatefilter.h
	photodither.h
	convolutionfilter.h
	desaturatefilter.h
	gaussfilter.h
	grayfilter.h
	invertfilter.h
	normalizefilter.h
	punchfilter.h
	sepiafilter.h
	cinemafilter.h
        oilpaintfilter.h
	frostedglassfilter.h
	posterizefilter.h
	vintagefilter.h
	filmgrainfilter.h
	qtimagefilter.h
	noise.h
	qtmatrix.h
	removecolorfilter.h
	thresholdfilter.h
	warholfilter.h
	colorconversions.h
	highpassfilter.h
	curvesfilter.h
	grittyfilter.h
	blendingmodes.h
	noisefilter.h
	autocontrastfilter.h
    texturefilter.h
	defocusfilter.h
)

SET(QtImageFilters_RSCS 
        qtimagefilters.qrc
)
QT4_ADD_RESOURCES(QtImageFilters_RSCS_SRCS ${QtImageFilters_RSCS})

INCLUDE_DIRECTORIES(
)


KDE4_ADD_LIBRARY(qtimagefilters ${qtimagefilters_SOURCES} ${qtimagefilters_HEADERS}  ${QtImageFilters_RSCS_SRCS})
IF(UNIX AND NOT APPLE)
	SET_TARGET_PROPERTIES( qtimagefilters PROPERTIES COMPILE_FLAGS -fPIC)
ENDIF(UNIX AND NOT APPLE)
TARGET_LINK_LIBRARIES(qtimagefilters ${QT_LIBRARIES})

