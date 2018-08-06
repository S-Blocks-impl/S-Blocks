#ifndef ENCLAVE_INTEGERS_HH
#define ENCLAVE_INTEGERS_HH
#include "../include/config.h"
#include "./glue.h"
#include "../include/type_traits.h"



/** @brief Return @a a / @a b. */
inline uint32_t int_divide(uint32_t a, uint32_t b) {
    return a / b;
}

/** @overload */
inline int32_t int_divide(int32_t a, uint32_t b) {
    return a / b;
}

#if HAVE_INT64_TYPES
/** @overload */
inline uint64_t int_divide(uint64_t a, uint32_t b) {
# if CLICK_LINUXMODULE && BITS_PER_LONG < 64
    do_div(a, b);
    return a;
# else
    return a / b;
# endif
}

/** @overload */
inline int64_t int_divide(int64_t a, uint32_t b) {
# if CLICK_LINUXMODULE && BITS_PER_LONG < 64
    if (unlikely(a < 0)) {
        uint64_t a_abs = -(a + 1);
        do_div(a_abs, b);
        return (int64_t) -a_abs - 1;
    } else {
        uint64_t &a_unsigned = reinterpret_cast<uint64_t &>(a);
        do_div(a_unsigned, b);
        return a_unsigned;
    }
# else
    return a / b;
# endif
}

/** @brief Multiply @a a * @a b, placing the low-order bits of the result in @a xlow
    and the high-order bits in @a xhigh. */
template<typename T>
void int_multiply(T a, T b, T &xlow, T &xhigh)
{
    typedef fast_half_integer<T> fasthalf;
    typedef typename fasthalf::half_type half_type;

    half_type al = fasthalf::low(a), ah = fasthalf::high(a),
        bl = fasthalf::low(b), bh = fasthalf::high(b);

    T r0 = T(al) * bl;
    T r3 = T(ah) * bh;
    T r1 = T(ah) * bl;
    T r2 = T(al) * bh + fasthalf::high(r0) + r1;
    if (r2 < r1)
        r3 += fasthalf::half_value;

    xhigh = r3 + fasthalf::high(r2);
    xlow = (r2 << fasthalf::half_bits) + fasthalf::low(r0);
}



/** @brief Divide @a a / @a b, placing quotient in @a quot and returning remainder. */
inline uint32_t int_remainder(uint32_t a, uint32_t b, uint32_t &quot) {
    quot = a / b;
    return a - quot * b;
}

/** @overload */
inline int32_t int_remainder(int32_t a, uint32_t b, int32_t &quot) {
    if (unlikely(a < 0))
        quot = -(-(a + 1) / b) - 1;
    else
        quot = a / b;
    return a - quot * b;
}

/** @overload */
inline uint32_t int_remainder(uint64_t a, uint32_t b, uint64_t &quot) {
# if CLICK_LINUXMODULE && BITS_PER_LONG < 64
    uint32_t rem = do_div(a, b);
    quot = a;
    return rem;
# else
    quot = a / b;
    return a - quot * b;
# endif
}

/** @overload */
inline uint32_t int_remainder(int64_t a, uint32_t b, int64_t &quot) {
# if CLICK_LINUXMODULE && BITS_PER_LONG < 64
    if (unlikely(a < 0)) {
        uint64_t a_abs = -(a + 1);
        uint32_t rem = do_div(a_abs, b);
        quot = (int64_t) -a_abs - 1;
        return rem ? b - rem : 0;
    } else {
        uint64_t &a_unsigned = reinterpret_cast<uint64_t &>(a);
        uint32_t rem = do_div(a_unsigned, b);
        quot = a_unsigned;
        return rem;
    }
# else
    // This arithmetic is about twice as fast on my laptop as the
    // alternative "div = a / b;
    //          rem = a - (value_type) div * b;
    //          if (rem < 0) div--, rem += b;",
    // and 3-4x faster than "div = a / b;
    //                   rem = a % b;
    //                   if (rem < 0) div--, rem += b;".
    if (unlikely(a < 0))
        quot = -(-(a + 1) / b) - 1;
    else
        quot = a / b;
    return a - quot * b;
# endif
}
#endif



// MSB is bit #1
#if HAVE___BUILTIN_CLZ && !HAVE_NO_INTEGER_BUILTINS
/** @brief Return the index of the most significant bit set in @a x.
 * @return 0 if @a x = 0; otherwise the index of first bit set, where the
 * most significant bit is numbered 1.
 */
inline int ffs_msb(unsigned x) {
    return (x ? __builtin_clz(x) + 1 : 0);
}
#else
# define NEED_FFS_MSB_UNSIGNED 1
/** @overload */
int ffs_msb(unsigned x);
#endif

#if HAVE___BUILTIN_CLZL && !HAVE_NO_INTEGER_BUILTINS
/** @overload */
inline int ffs_msb(unsigned long x) {
    return (x ? __builtin_clzl(x) + 1 : 0);
}
#elif SIZEOF_INT == SIZEOF_LONG
/** @overload */
inline int ffs_msb(unsigned long x) {
    return ffs_msb(static_cast<unsigned>(x));
}
#else
# define NEED_FFS_MSB_UNSIGNED_LONG 1
/** @overload */
int ffs_msb(unsigned long x);
#endif

#if HAVE_LONG_LONG && HAVE___BUILTIN_CLZLL && !HAVE_NO_INTEGER_BUILTINS
/** @overload */
inline int ffs_msb(unsigned long long x) {
    return (x ? __builtin_clzll(x) + 1 : 0);
}
#elif HAVE_LONG_LONG && SIZEOF_LONG == SIZEOF_LONG_LONG
/** @overload */
inline int ffs_msb(unsigned long long x) {
    return ffs_msb(static_cast<unsigned long>(x));
}
#elif HAVE_LONG_LONG
# define NEED_FFS_MSB_UNSIGNED_LONG_LONG 1
/** @overload */
int ffs_msb(unsigned long long x);
#endif

#if HAVE_INT64_TYPES && !HAVE_INT64_IS_LONG && !HAVE_INT64_IS_LONG_LONG
# if SIZEOF_LONG >= 8
/** @overload */
inline int ffs_msb(uint64_t x) {
    return ffs_msb(static_cast<unsigned long>(x));
}
# elif HAVE_LONG_LONG && SIZEOF_LONG_LONG >= 8
/** @overload */
inline int ffs_msb(uint64_t x) {
    return ffs_msb(static_cast<unsigned long long>(x));
}
# else
#  define NEED_FFS_MSB_UINT64_T 1
/** @overload */
int ffs_msb(uint64_t x);
# endif
#endif


// LSB is bit #1
#if HAVE___BUILTIN_FFS && !HAVE_NO_INTEGER_BUILTINS
/** @brief Return the index of the least significant bit set in @a x.
 * @return 0 if @a x = 0; otherwise the index of first bit set, where the
 * least significant bit is numbered 1.
 */
inline int ffs_lsb(unsigned x) {
    return __builtin_ffs(x);
}
#elif HAVE_FFS && !HAVE_NO_INTEGER_BUILTINS
/** overload */
inline int ffs_lsb(unsigned x) {
    return ffs(x);
}
#else
# define NEED_FFS_LSB_UNSIGNED 1
/** @overload */
int ffs_lsb(unsigned x);
#endif

#if HAVE___BUILTIN_FFSL && !HAVE_NO_INTEGER_BUILTINS
/** @overload */
inline int ffs_lsb(unsigned long x) {
    return __builtin_ffsl(x);
}
#elif SIZEOF_INT == SIZEOF_LONG
/** @overload */
inline int ffs_lsb(unsigned long x) {
    return ffs_lsb(static_cast<unsigned>(x));
}
#else
# define NEED_FFS_LSB_UNSIGNED_LONG 1
/** @overload */
int ffs_lsb(unsigned long x);
#endif

#if HAVE_LONG_LONG && HAVE___BUILTIN_FFSLL && !HAVE_NO_INTEGER_BUILTINS
/** @overload */
inline int ffs_lsb(unsigned long long x) {
    return __builtin_ffsll(x);
}
#elif HAVE_LONG_LONG && SIZEOF_LONG == SIZEOF_LONG_LONG
/** @overload */
inline int ffs_lsb(unsigned long long x) {
    return ffs_lsb(static_cast<unsigned long>(x));
}
#elif HAVE_LONG_LONG
# define NEED_FFS_LSB_UNSIGNED_LONG_LONG 1
/** @overload */
int ffs_lsb(unsigned long long x);
#endif

#if HAVE_INT64_TYPES && !HAVE_INT64_IS_LONG && !HAVE_INT64_IS_LONG_LONG
# if SIZEOF_LONG >= 8
/** @overload */
inline int ffs_lsb(uint64_t x) {
    return ffs_lsb(static_cast<unsigned long>(x));
}
# elif HAVE_LONG_LONG && SIZEOF_LONG_LONG >= 8
/** @overload */
inline int ffs_lsb(uint64_t x) {
    return ffs_lsb(static_cast<unsigned long long>(x));
}
# else
#  define NEED_FFS_LSB_UINT64_T 1
/** @overload */
int ffs_lsb(uint64_t x);
# endif
#endif


#endif
