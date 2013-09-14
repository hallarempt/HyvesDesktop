SET(SOURCES
	GenusApis.cpp
	OAuthAccessToken.cpp
	OAuthBase.cpp
	OAuthConsumer.cpp
	OAuthRequestToken.cpp
	OAuthUtil.cpp
	ResultDocument.cpp
)

SET(HEADERS
	OAuthAccessToken.h
	OAuthBase.h
	OAuthConsumer.h
	OAuthRequestToken.h
	OAuthUtil.h
	ResultDocument.h
	GenusApis.h
)

INCLUDE_DIRECTORIES(
	${QCA2_INCLUDE_DIR}
)


KDE4_ADD_LIBRARY(genusapis ${SOURCES} ${HEADERS})
IF(UNIX AND NOT APPLE)
	SET_TARGET_PROPERTIES( genusapis PROPERTIES COMPILE_FLAGS -fPIC)
ENDIF(UNIX AND NOT APPLE)
TARGET_LINK_LIBRARIES(genusapis ${QT_LIBRARIES} ${QCA2_LIBRARIES})

