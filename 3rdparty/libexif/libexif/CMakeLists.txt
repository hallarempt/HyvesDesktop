IF(WIN32)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/_config-exif-win.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/_config-exif.h)
ELSE(WIN32)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/_config-exif.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/_config-exif.h)
ENDIF(WIN32)

SET( libexif_SRCS exif-byte-order.c
	exif-content.c
	exif-data.c
	exif-entry.c
	exif-format.c
	exif-ifd.c
	exif-loader.c
	exif-log.c
	exif-mem.c
	exif-mnote-data.c
	exif-tag.c
	exif-utils.c
	canon/exif-mnote-data-canon.c
	canon/mnote-canon-entry.c
	canon/mnote-canon-tag.c
	fuji/exif-mnote-data-fuji.c
	fuji/mnote-fuji-entry.c
	fuji/mnote-fuji-tag.c
	olympus/exif-mnote-data-olympus.c
	olympus/mnote-olympus-entry.c
	olympus/mnote-olympus-tag.c
	pentax/exif-mnote-data-pentax.c
	pentax/mnote-pentax-entry.c
	pentax/mnote-pentax-tag.c

)
add_library(exif ${libexif_SRCS})
target_link_libraries( exif ${QT_LIBRARIES})

IF(UNIX AND NOT APPLE)
SET_TARGET_PROPERTIES(exif PROPERTIES COMPILE_FLAGS -fPIC)
ENDIF(UNIX AND NOT APPLE)
