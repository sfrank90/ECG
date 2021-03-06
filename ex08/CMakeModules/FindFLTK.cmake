# -Try to find FLTK 
# 
#
# The following are set after configuration is done:
# FLTK_INCLUDE_DIR
# FLTK_FOUND
# FLTK_LIBRARIES
# FLTK_LIBRARY
# FLTK_GL_LIBRARY
# FLTK_IMAGES_LIBRARY

FIND_PATH(FLTK_INCLUDE_DIR FL/Fl.h
  /usr/include
  /usr/local/include
)

SET(FLTK_POSSIBLE_LIBPATHS
  /usr/lib
  /usr/local/lib
  /usr/lib64
  /usr/local/lib64
)

SET(FLTK_FOUND TRUE)

FIND_LIBRARY(FLTK_LIBRARY
  NAMES fltk
  PATHS ${FLTK_POSSIBLE_LIBPATHS}
)

IF(NOT FLTK_LIBRARY)
       MESSAGE(SEND_ERROR "fltk library not found.")
       SET(FLTK_FOUND FALSE)
ENDIF(NOT FLTK_LIBRARY)

FIND_LIBRARY(FLTK_GL_LIBRARY
  NAMES fltk_gl
  PATHS ${FLTK_POSSIBLE_LIBPATHS}
)

IF(NOT FLTK_GL_LIBRARY)
       MESSAGE(SEND_ERROR "fltk_gl library not found.")
       SET(FLTK_FOUND FALSE)
ENDIF(NOT FLTK_GL_LIBRARY)

FIND_LIBRARY(FLTK_IMAGES_LIBRARY
  NAMES fltk_images
  PATHS ${FLTK_POSSIBLE_LIBPATHS}
)

IF(NOT FLTK_IMAGES_LIBRARY)
       MESSAGE(SEND_ERROR "fltk_images library not found.")
       SET(FLTK_FOUND FALSE)
ENDIF(NOT FLTK_IMAGES_LIBRARY)

IF(FLTK_FOUND)
	MESSAGE(STATUS "Found fltk library")
	SET(FLTK_LIBRARIES ${FLTK_LIBRARY} ${FLTK_GL_LIBRARY} ${FLTK_IMAGES_LIBRARY} )
ENDIF(FLTK_FOUND)

MARK_AS_ADVANCED(
  FLTK_FOUND
  FLTK_LIBRARY
  FLTK_GL_LIBRARY
  FLTK_IMAGES_LIBRARY
)