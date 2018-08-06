#include "../include/config.h"
#include "integer.h"
// #define  NEED_FFS_MSB_UNSIGNED  1
// #undef NEED_FFS_MSB_UNSIGNED_LONG 
// #undef NEED_FFS_MSB_UNSIGNED_LONG_LONG

#if NEED_FFS_MSB_UNSIGNED
int
ffs_msb(unsigned x)
{
    static_assert(sizeof(unsigned) == 4, "unsigned has unexpected size.");

    int add = 0;
    static uint8_t bvals[] = { 0, 4, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1 };

    if ((x & 0xFFFF0000) == 0) {
        if (x == 0)             /* zero input ==> zero output */
            return 0;
        add += 16;
    } else
        x >>= 16;

    if ((x & 0xFF00) == 0)
        add += 8;
    else
        x >>= 8;

    if ((x & 0xF0) == 0)
        add += 4;
    else
        x >>= 4;

    return add + bvals[x & 0xF];
}
#endif

#define ffs_msb_hilo(type)                      \
    size_t bits = sizeof(type) * 8;             \
    type hi = (type) (x >> bits);               \
    if (hi == 0) {                              \
        type lo = (type) x;                     \
        return lo ? bits + ffs_msb(lo) : 0;     \
    } else                                      \
        return ffs_msb(hi);

#if NEED_FFS_MSB_UNSIGNED_LONG
int
ffs_msb(unsigned long x)
{
    static_assert(sizeof(unsigned) == 4 && sizeof(unsigned long) == 8, "unsigned or unsigned long has unexpected size.");
    ffs_msb_hilo(unsigned);
}
#endif

#if NEED_FFS_MSB_UNSIGNED_LONG_LONG
int
ffs_msb(unsigned long long x)
{
# if SIZEOF_LONG_LONG == 8
    static_assert(sizeof(unsigned) == 4, "unsigned has unexpected size.");
    ffs_msb_hilo(unsigned);
# elif SIZEOF_LONG_LONG == 16
    static_assert(sizeof(unsigned long) == 8, "unsigned long has unexpected size.");
    ffs_msb_hilo(unsigned long);
# else
    static_assert(sizeof(unsigned long long) == 8 || sizeof(unsigned long long) == 16, "unsigned long long has unexpected size.");
# endif
}
#endif

#if NEED_FFS_MSB_UINT64_T
int
ffs_msb(uint64_t x)
{
    static_assert(sizeof(unsigned) == 4, "unsigned has unexpected size.");
    ffs_msb_hilo(unsigned);
}
#endif


#if NEED_FFS_LSB_UNSIGNED
int
ffs_lsb(uint32_t x)
{
    static_assert(sizeof(unsigned) == 4, "unsigned has unexpected size.");

    int add = 0;
    static uint8_t bvals[] = { 0, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1 };

    if ((x & 0x0000FFFF) == 0) {
        if (x == 0)             /* zero input ==> zero output */
            return 0;
        add += 16;
        x >>= 16;
    }

    if ((x & 0x00FF) == 0) {
        add += 8;
        x >>= 8;
    }

    if ((x & 0x0F) == 0) {
        add += 4;
        x >>= 4;
    }

    return add + bvals[x & 0xF];
}
#endif

#define ffs_lsb_hilo(type)                      \
    size_t bits = sizeof(type) * 8;             \
    type lo = (type) x;                         \
    if (lo == 0) {                              \
        type hi = (type) (x >> bits);           \
        return hi ? bits + ffs_lsb(hi) : 0;     \
    } else                                      \
        return ffs_lsb(lo);

#if NEED_FFS_LSB_UNSIGNED_LONG
int
ffs_lsb(unsigned long x)
{
    static_assert(sizeof(unsigned) == 4 && sizeof(unsigned long) == 8, "unsigned or unsigned long has unexpected size.");
    ffs_lsb_hilo(unsigned);
}
#endif

#if NEED_FFS_LSB_UNSIGNED_LONG_LONG
int
ffs_lsb(unsigned long long x)
{
# if SIZEOF_LONG_LONG == 8
    static_assert(sizeof(unsigned) == 4, "unsigned has unexpected size.");
    ffs_lsb_hilo(unsigned);
# elif SIZEOF_LONG_LONG == 16
    static_assert(sizeof(unsigned long) == 8, "unsigned long has unexpected size.");
    ffs_lsb_hilo(unsigned long);
# else
    static_assert(sizeof(unsigned long long) == 8 || sizeof(unsigned long long) == 16, "unsigned long long has unexpected size.");
# endif
}
#endif

#if NEED_FFS_LSB_UINT64_T
int
ffs_lsb(uint64_t x)
{
    static_assert(sizeof(unsigned) == 4, "unsigned has unexpected size.");
    ffs_lsb_hilo(unsigned);
}
#endif


