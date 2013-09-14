# Copy Qt DLLs
INSTALL(FILES
	"${QT_BINARY_DIR}/QtCore${QT_VERSION_MAJOR}.dll"
	"${QT_BINARY_DIR}/QtXml${QT_VERSION_MAJOR}.dll"
	"${QT_BINARY_DIR}/QtTest${QT_VERSION_MAJOR}.dll"
	"${QT_BINARY_DIR}/QtWebKit${QT_VERSION_MAJOR}.dll"
	"${QT_BINARY_DIR}/QtGui${QT_VERSION_MAJOR}.dll"
	"${QT_BINARY_DIR}/QtNetwork${QT_VERSION_MAJOR}.dll"
	"${QT_BINARY_DIR}/QtSvg${QT_VERSION_MAJOR}.dll"
	"${QT_BINARY_DIR}/qca.dll"
	DESTINATION bin
)

# Copy Qt plugins
FOREACH(PLUGIN
	"accessible/qtaccessiblecompatwidgets4"
	"accessible/qtaccessiblewidgets4"
	"codecs/qcncodecs4"
	"codecs/qjpcodecs4"
	"codecs/qkrcodecs4"
	"codecs/qtwcodecs4"
	"crypto/qca-ossl2"
	"iconengines/qsvgicon4"
	"imageformats/qgif4"
	"imageformats/qico4"
	"imageformats/qjpeg4"
	"imageformats/qsvg4"
)
	STRING(REGEX REPLACE "/.*" "" PLUGINDIR "${PLUGIN}")
	INSTALL(FILES "${QT_BINARY_DIR}/../plugins/${PLUGIN}.dll"
		DESTINATION "plugins/${PLUGINDIR}"
	)
ENDFOREACH(PLUGIN)

# Copy MSI and MSVC redistributables
INSTALL(FILES
	vcredist_x86.exe
	DESTINATION bin
)

# Copy OpenSSL DLLs
FOREACH(SSLLIB libeay32 libssl32 ssleay32)
	INSTALL(FILES "C:/OpenSSL/${SSLLIB}.dll" DESTINATION bin)
ENDFOREACH(SSLLIB)

# Copy ZLib DLL
INSTALL(FILES "${ZLIB_INCLUDE_DIR}/../zlib1.dll" DESTINATION bin)
