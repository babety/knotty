#ifndef XSVER_H_
#define XSVER_H_ 1

#include "xsdef.h"

#define XSLIB_EDITION	181103
#define XSLIB_REVISION	181103
#define XSLIB_RELEASE	20

#define XSLIB_VERSION	XS_TOSTR(XSLIB_EDITION) "." XS_TOSTR(XSLIB_REVISION) "." XS_TOSTR(XSLIB_RELEASE)


#ifdef __cplusplus
extern "C" {
#endif


const char *xslib_version_string();

const char *xslib_version_rcsid();


#ifdef __cplusplus
}
#endif

#endif

