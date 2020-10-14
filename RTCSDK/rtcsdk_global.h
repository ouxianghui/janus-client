/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

//#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(RTCSDK_LIB)
#  define RTCSDK_EXPORT Q_DECL_EXPORT
# else
#  define RTCSDK_EXPORT Q_DECL_IMPORT
# endif
#else
# define RTCSDK_EXPORT
#endif
