#ifndef _ENCLAVE_CONF_H
#define _ENCLAVE_CONF_H

#define CLICK_USERLEVEL 1
#undef CLICK_LINUXMODULE
#undef CLICK_BSDMODULE
#undef CLICK_NS

#define HAVE_BATCH 1
#define HAVE_CLICK_PACKET_POOL 1
//#define HAVE_AUTO_BATCH 1

#define DEBUG_PRINT 
#ifdef DEBUG_PRINT
#define PRINT(data)  ocall_print(data);
#else
#define PRINT(data) 
#endif
#include "../enclave_t.h"
//====================config.h==================

#define CLICK_BIG_ENDIAN    4321
#define CLICK_LITTLE_ENDIAN 1234
#define CLICK_NO_ENDIAN     0

/* Define to byte order of target machine. */
#define CLICK_BYTE_ORDER CLICK_LITTLE_ENDIAN


/* Define ARCH_IS_BIG_ENDIAN based on CLICK_BYTE_ORDER. */
#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
# define ARCH_IS_BIG_ENDIAN 1
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
# define ARCH_IS_BIG_ENDIAN 0
#endif


/* Define if the C++ compiler understands rvalue references. */
//#define HAVE_CXX_RVALUE_REFERENCES 1

/* Define if you have the __has_trivial_copy compiler intrinsic. */
#define HAVE___HAS_TRIVIAL_COPY 1



/* Define if the machine is indifferent to alignment. */
#define HAVE_INDIFFERENT_ALIGNMENT 1

/* Define if 64-bit integer types are enabled. */
#define HAVE_INT64_TYPES 1




/* Floating point arithmetic is allowed. */
#define HAVE_FLOAT_TYPES 1
/* Define to 1 if the system has the type `long long'. */
#define HAVE_LONG_LONG 1

//define the size from click/config.h 
/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG 8

/* The size of a `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of a `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T 8

/* The size of a `struct timespec', as computed by sizeof. */
#define SIZEOF_STRUCT_TIMESPEC 16

/* The size of a `struct timeval', as computed by sizeof. */
#define SIZEOF_STRUCT_TIMEVAL 16

/* The size of a `ptrdiff_t', as computed by sizeof. */
#define SIZEOF_PTRDIFF_T 8

/* The size of a `void *', as computed by sizeof. */
#define SIZEOF_VOID_P 8


//attributes
/* Define macro for deprecated functions. */
#if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ == 0)
# define CLICK_DEPRECATED /* nothing */
#else
# define CLICK_DEPRECATED __attribute__((deprecated))
#endif

/* Define macro for deprecated functions with message. */
#if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ == 0)
# define CLICK_DEPRECATED_MSG(m) /* nothing */
#elif __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ <= 4)
# define CLICK_DEPRECATED_MSG(m) __attribute__((deprecated))
#else
# define CLICK_DEPRECATED_MSG(m) __attribute__((deprecated(m)))
#endif

/* Define macro for deprecated enumerations. */
#if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
# define CLICK_DEPRECATED_ENUM /* nothing */
#else
# define CLICK_DEPRECATED_ENUM __attribute__((deprecated))
#endif

/* Define macros for marking types as may-alias. */
#if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 3)
# define CLICK_MAY_ALIAS /* nothing */
#else
# define CLICK_MAY_ALIAS __attribute__((__may_alias__))
#endif

/* Define macro for marking functions noninlinable. */
#ifdef CLICK_LINUXMODULE
# define CLICK_NOINLINE noinline
#elif __GNUC__
# define CLICK_NOINLINE __attribute__((noinline))
#else
# define CLICK_NOINLINE /* nothing */
#endif

/* Define macro for funtions that should be inline-able even if compiling without optimization. */
#if __GNUC__
# define CLICK_ALWAYS_INLINE __attribute__((always_inline))
#else
# define CLICK_ALWAYS_INLINE /* nothing */
#endif

/* Define macros for declaring packed structures. */
#ifdef __GNUC__
# define CLICK_PACKED_STRUCTURE(open, close) open close __attribute__((packed))
# define CLICK_SIZE_PACKED_STRUCTURE(open, close) open close __attribute__((packed)) /* deprecated */
# define CLICK_SIZE_PACKED_ATTRIBUTE __attribute__((packed))
#else
# define CLICK_PACKED_STRUCTURE(open, close) _Cannot_pack_structure__Use_GCC
# define CLICK_SIZE_PACKED_STRUCTURE(open, close) open close /* deprecated */
# define CLICK_SIZE_PACKED_ATTRIBUTE
#endif

/* Define macro for functions whose results should not be ignored. */
#if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
# define CLICK_WARN_UNUSED_RESULT /* nothing */
#else
# define CLICK_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#endif

/* Define macro for cold (rarely used) functions. */
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 3)
# define CLICK_COLD /* nothing */
#else
# define CLICK_COLD __attribute__((cold))
#endif




//====================end of config.h==================


//====================config-userlevel.h==============



#if __GNUC__ >= 3
# define likely(x)  __builtin_expect(!!(x), 1)
# define unlikely(x)    __builtin_expect(!!(x), 0)
#else
# define likely(x)  (x)
# define unlikely(x)    (x)
#endif

/* Define if 'int64_t' is typedefed to 'long' at user level. */
#define HAVE_INT64_IS_LONG_USERLEVEL 1

/* Define HAVE_INT64_IS_LONG based on HAVE_INT64_IS_LONG_USERLEVEL. */
#ifdef HAVE_INT64_IS_LONG_USERLEVEL
# define HAVE_INT64_IS_LONG HAVE_INT64_IS_LONG_USERLEVEL
#endif

/* Define HAVE_INT64_IS_LONG_LONG based on HAVE_INT64_IS_LONG_LONG_USERLEVEL. */
#ifdef HAVE_INT64_IS_LONG_LONG_USERLEVEL
# define HAVE_INT64_IS_LONG_LONG HAVE_INT64_IS_LONG_LONG_USERLEVEL
#endif

/* Define if a Click user-level driver might run multiple threads. */
#undef HAVE_USER_MULTITHREAD

#ifdef HAVE_USER_MULTITHREAD
# define HAVE_MULTITHREAD HAVE_USER_MULTITHREAD
#endif

#include <sys/types.h>
/* Define uint types in terms of u_int types, if necessary. */
#ifdef HAVE_U_INT_TYPES
typedef u_int8_t uint8_t;
typedef u_int16_t uint16_t;
typedef u_int32_t uint32_t;
# ifdef HAVE_INT64_TYPES
typedef u_int64_t uint64_t;
# endif
typedef long intptr_t;      /* XXX? */
typedef unsigned long uintptr_t;
#endif


#include <assert.h>
//====================end of config-userlevel.h==============




//====================onfig.h==================

/* Ensure declaration of DefaultArg template. */
#ifdef __cplusplus

template <typename T> struct DefaultArg;

/** @class uninitialized_type
    @brief Type tag indicating an object should not be initialized. */
struct uninitialized_type {
};

#endif

/* Define aliasing versions of integer and pointer types. */
typedef uint16_t click_aliasable_uint16_t CLICK_MAY_ALIAS;
typedef int16_t click_aliasable_int16_t CLICK_MAY_ALIAS;
typedef uint32_t click_aliasable_uint32_t CLICK_MAY_ALIAS;
typedef int32_t click_aliasable_int32_t CLICK_MAY_ALIAS;
#if HAVE_INT64_TYPES
typedef uint64_t click_aliasable_uint64_t CLICK_MAY_ALIAS;
typedef int64_t click_aliasable_int64_t CLICK_MAY_ALIAS;
#endif
typedef void *click_aliasable_void_pointer_t CLICK_MAY_ALIAS;

//====================end of config.h==================


//====================from packet.hh===================
#ifndef CLICK_PACKET_DEPRECATED_ENUM
# define CLICK_PACKET_DEPRECATED_ENUM CLICK_DEPRECATED_ENUM
#endif

#define HAVE_CLICK_PACKET_POOL 1
//====================end of from packet.hh============





/* Define to 1 if the system has the type `struct timespec'. */
#undef HAVE_STRUCT_TIMESPEC



typedef long int __time_t;  
typedef unsigned int __suseconds_t;

struct timeval
{
    __time_t tv_sec;        /* Seconds.  */
    __suseconds_t tv_usec;  /* Microseconds.  */
};


#define ENOMEM          12      /* Out of memory */
#define EACCES          13      /* Permission denied */
#define EINVAL          22      /* Invalid argument */
#define ERANGE          34      /* Math result not representable */



#endif
