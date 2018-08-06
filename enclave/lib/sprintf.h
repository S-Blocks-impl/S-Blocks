#ifndef __ENCLAVE_SPRINTF_H
#define __ENCLAVE_SPRINTF_H
#include <stdarg.h>


#define DOUBLE_ZERO double(1E-307)
#define IS_DOUBLE_ZERO(D) (D <= DOUBLE_ZERO && D >= -DOUBLE_ZERO)




int sprintf(char* buf, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);

#endif