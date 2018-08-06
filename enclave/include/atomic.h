#ifndef _ATOMIC_H
#define _ATOMIC_H

#define CLICK_ATOMIC_LOCK lock

class atomic_uint32_t
{
public:
    inline uint32_t value() const;    
    inline operator uint32_t() const;

    inline atomic_uint32_t& operator= (uint32_t x);
    inline atomic_uint32_t& operator+= (uint32_t delta);
    inline atomic_uint32_t& operator-= (uint32_t delta);
    inline atomic_uint32_t& operator|= (uint32_t mask);
    inline atomic_uint32_t& operator&= (uint32_t mask);

    inline void operator++();
    inline void operator--();
    inline void operator++(int);
    inline void operator--(int);

    inline  static bool dec_and_test(volatile uint32_t &x);
    inline   bool dec_and_test();
    inline  static void inc(volatile uint32_t &x);
    inline static uint32_t compare_swap(volatile uint32_t &x, uint32_t expected, uint32_t desired);

private:
    uint32_t _val;

};

inline uint32_t
atomic_uint32_t::value() const
{
    return _val;
}

inline
atomic_uint32_t::operator uint32_t() const
{
    return value();
}


inline atomic_uint32_t&
atomic_uint32_t::operator=
 (uint32_t x)
{
    _val = x;
    return *this;
}

inline atomic_uint32_t&
atomic_uint32_t::operator+= (uint32_t delta)
{

#if HAVE_MULTITHREAD
    asm volatile (CLICK_ATOMIC_LOCK "addl %1,%0"
              : "=m" (CLICK_ATOMIC_VAL)
              : "r" (delta), "m" (CLICK_ATOMIC_VAL)
              : "cc");
#else
    _val = _val + delta;
#endif
    return *this;
}

inline atomic_uint32_t&
atomic_uint32_t::operator-= (uint32_t delta)
{
#if HAVE_MULTITHREAD
    asm volatile (CLICK_ATOMIC_LOCK "subl %1,%0"
		  : "=m" (CLICK_ATOMIC_VAL)
		  : "r" (delta), "m" (CLICK_ATOMIC_VAL)
		  : "cc");
#else
    _val = _val - delta;
#endif
    return *this;
}

inline atomic_uint32_t&
atomic_uint32_t::operator|= (uint32_t mask)
{
#if HAVE_MULTITHREAD
    asm volatile (CLICK_ATOMIC_LOCK "orl %1,%0"
		  : "=m" (CLICK_ATOMIC_VAL)
		  : "r" (mask), "m" (CLICK_ATOMIC_VAL)
		  : "cc");
#else
    _val |= mask;
#endif
   return *this;
}

inline atomic_uint32_t&
atomic_uint32_t::operator&= (uint32_t mask)
{
#if HAVE_MULTITHREAD
    asm volatile (CLICK_ATOMIC_LOCK "andl %1,%0"
		  : "=m" (CLICK_ATOMIC_VAL)
		  : "r" (mask), "m" (CLICK_ATOMIC_VAL)
		  : "cc");
#else
    _val &= mask;
#endif
   return *this;
}

/** @brief  Atomically increment value @a x. */
inline void
atomic_uint32_t::inc(volatile uint32_t &x)
{

#if HAVE_MULTITHREAD
    asm volatile (CLICK_ATOMIC_LOCK "incl %0"
		  : "=m" (x)
		  : "m" (x)
		  : "cc");
#else
    x++;
#endif
}


inline bool 
atomic_uint32_t::dec_and_test(volatile uint32_t &x)
{
#if HAVE_MULTITHREAD    
    uint8_t result;
    asm volatile (CLICK_ATOMIC_LOCK "decl %0 ; sete %1"
		  : "=m" (x), "=qm" (result)
		  : "m" (x)
		  : "cc");
    return result;
#else
    return (--x == 0);
#endif
}

inline bool
atomic_uint32_t::dec_and_test()
{

#if HAVE_MULTITHREAD
    uint8_t result;
    asm volatile (CLICK_ATOMIC_LOCK "decl %0 ; sete %1"
		  : "=m" (CLICK_ATOMIC_VAL), "=qm" (result)
		  : "m" (CLICK_ATOMIC_VAL)
		  : "cc");
    return result;
#else
    return (--_val == 0);
#endif

}

/** @brief  Perform a compare-and-swap operation.
 *  @param  x         value
 *  @param  expected  test value
 *  @param  desired   new value
 *  @return The actual old value.  If it equaled @a expected, @a x has been
 *	    set to @a desired.
 *
 * Behaves like this, but in one atomic step:
 * @code
 * uint32_t actual = x;
 * if (x == expected)
 *     x = desired;
 * return actual;
 * @endcode
 *
 * Also acts as a memory barrier. */
inline uint32_t
atomic_uint32_t::compare_swap(volatile uint32_t &x, uint32_t expected, uint32_t desired)
{
#if HAVE_MULTITHREAD
    asm volatile (CLICK_ATOMIC_LOCK "cmpxchgl %2,%1"
		  : "=a" (expected), "=m" (x)
		  : "r" (desired), "0" (expected), "m" (x)
		  : "cc", "memory");
    return expected;
#else
    uint32_t actual = x;
    if (actual == expected)
	x = desired;
    return actual;
#endif
}


inline void
atomic_uint32_t::operator++()
{
#if HAVE_MULTITHREAD
    asm volatile (CLICK_ATOMIC_LOCK "incl %0"
		  : "=m" (CLICK_ATOMIC_VAL)
		  : "m" (CLICK_ATOMIC_VAL)
		  : "cc");
#else
    _val++;
#endif
}

/** @brief  Atomically increment the value. */
inline void
atomic_uint32_t::operator++(int)
{
#if HAVE_MULTITHREAD
    asm volatile (CLICK_ATOMIC_LOCK "incl %0"
		  : "=m" (CLICK_ATOMIC_VAL)
		  : "m" (CLICK_ATOMIC_VAL)
		  : "cc");
#else
    _val++;
#endif
}

/** @brief  Atomically decrement the value. */
inline void
atomic_uint32_t::operator--()
{
#if HAVE_MULTITHREAD

    asm volatile (CLICK_ATOMIC_LOCK "decl %0"
		  : "=m" (CLICK_ATOMIC_VAL)
		  : "m" (CLICK_ATOMIC_VAL)
		  : "cc");
#else
    _val--;
#endif
}

/** @brief  Atomically decrement the value. */
inline void
atomic_uint32_t::operator--(int)
{
#if HAVE_MULTITHREAD

    asm volatile (CLICK_ATOMIC_LOCK "decl %0"
		  : "=m" (CLICK_ATOMIC_VAL)
		  : "m" (CLICK_ATOMIC_VAL)
		  : "cc");
#else
    _val--;
#endif
}


#endif



