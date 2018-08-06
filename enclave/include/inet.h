#ifndef ENCLAVE_INET_H
#define ENCLAVE_INET_H

#define GCC_VERSION (__GNUC__ * 10000       \
             + __GNUC_MINOR__ * 100 \
             + __GNUC_PATCHLEVEL__)

#if GCC_VERSION >= 40400
#define __HAVE_BUILTIN_BSWAP32__
#define __HAVE_BUILTIN_BSWAP16__
#endif

#ifdef __HAVE_BUILTIN_BSWAP32__
#define __swab32(x) (__u32)__builtin_bswap32((__u32)(x))
#endif
#ifdef __HAVE_BUILTIN_BSWAP16__
#define __swab16(x) (__u16)__builtin_bswap16((__u16)(x))
#endif

#define __bitwise 
#define __force    __attribute__((force))

typedef unsigned int __u32;
typedef unsigned short __u16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __be32;

////////////add by rong//////
//#define cpu_to_be32 __cpu_to_be32

//////////////////////

#undef htonl
#undef ntohl
#undef htons
#undef ntohs

#define __cpu_to_be32(x) ((__force __be32)__swab32((x)))
#define __cpu_to_be16(x) ((__force __be16)__swab16((x)))
#define __be32_to_cpu(x) __swab32((__force __u32)(__be32)(x))
#define __be16_to_cpu(x) __swab16((__force __u16)(__be16)(x))

//htonl  unsigned long ,host to net : little end To big end 
//htons  unsigned short , host to net:  little end To big end
//ntohl unsigned long, net to host: big end To little end
//ntohs unsigned short , net to host: big end To little end

#define ___htonl(x) __cpu_to_be32(x)
#define ___htons(x) __cpu_to_be16(x)
#define ___ntohl(x) __be32_to_cpu(x)
#define ___ntohs(x) __be16_to_cpu(x)

#define htonl(x) ___htonl(x)
#define ntohl(x) ___ntohl(x)
#define htons(x) ___htons(x)
#define ntohs(x) ___ntohs(x)


#endif