#ifndef ENCLAVE_CONFPARSE_H
#define ENCLAVE_CONFPARSE_H

#include "String.h"
#include <vector>
class ErrorHandler;
class StringAccum;
class Timestamp;

const char* cp_process_backslash(const char* begin, const char* end, StringAccum &sa);

String cp_unquote(const String &str);


void cp_argvec(const String& str, stlpmtx_std::vector<String>& conf);


void cp_spacevec(const String& str, stlpmtx_std::vector<String>& conf);




/// @brief  Remove and return the first space-separated argument from @a str.
/// @param[in,out]  str  space-separated configuration string
///
/// The first space-separated argument in the configuration string is removed
/// and returned.  The returned argument is passed through cp_uncomment().  @a
/// str is set to the remaining portion of the string, with any preceding
/// spaces and comments removed.  If the input string is all spaces and
/// comments, then both the returned string and @a str will be empty.
String cp_shift_spacevec(String &str);


/// @name Direct Parsing Functions
//@{
enum CpErrors {
    CPE_OK = 0,
    CPE_FORMAT,
    CPE_NEGATIVE,
    CPE_OVERFLOW,
    CPE_INVALID,
    CPE_MEMORY,
    CPE_NOUNITS
};
extern int cp_errno;

// strings and words
bool cp_string(const String& str, String* result, String *rest = 0);








// numbers
enum { cp_basic_integer_whole = 64 };
const char *cp_basic_integer(const char *begin, const char *end, int flags, int size, void *result);

inline const char *cp_integer(const char *begin, const char *end, int base, int *result);
inline const char *cp_integer(const char *begin, const char *end, int base, unsigned *result);
inline const char *cp_integer(const char *begin, const char *end, int base, long *result);
inline const char *cp_integer(const char *begin, const char *end, int base, unsigned long *result);
/// @cond never
inline const unsigned char *cp_integer(const unsigned char *begin, const unsigned char *end, int base, unsigned *result);
inline const unsigned char *cp_integer(const unsigned char *begin, const unsigned char *end, int base, unsigned long *result);
/// @endcond
#if HAVE_LONG_LONG
inline const char *cp_integer(const char *begin, const char *end, int base, long long *result);
inline const char *cp_integer(const char *begin, const char *end, int base, unsigned long long *result);
/// @cond never
inline const unsigned char *cp_integer(const unsigned char *begin, const unsigned char *end, int base, unsigned long long *result);
/// @endcond
#elif HAVE_INT64_TYPES && !HAVE_INT64_IS_LONG
inline const char *cp_integer(const char *begin, const char *end, int base, int64_t *result);
inline const char *cp_integer(const char *begin, const char *end, int base, uint64_t *result);
/// @cond never
inline const unsigned char *cp_integer(const unsigned char *begin, const unsigned char *end, int base, uint64_t *result);
/// @endcond
#endif

bool cp_real10(const String& str, int frac_digits, int32_t* result);
bool cp_real10(const String& str, int frac_digits, uint32_t* result);
bool cp_real10(const String& str, int frac_digits, uint32_t* result_int, uint32_t* result_frac);


bool cp_time(const String &str, Timestamp *result, bool allow_negative = false);
bool cp_time(const String& str, struct timeval* result);




inline bool cp_integer(const String &str, int base, int *result);
inline bool cp_integer(const String &str, int base, unsigned int *result);
inline bool cp_integer(const String &str, int base, long *result);
inline bool cp_integer(const String &str, int base, unsigned long *result);
#if HAVE_LONG_LONG
inline bool cp_integer(const String &str, int base, long long *result);
inline bool cp_integer(const String &str, int base, unsigned long long *result);
#elif HAVE_INT64_TYPES && !HAVE_INT64_IS_LONG
inline bool cp_integer(const String &str, int base, int64_t *result);
inline bool cp_integer(const String &str, int base, uint64_t *result);
#endif

inline bool cp_integer(const String &str, int *result);
inline bool cp_integer(const String &str, unsigned int *result);
inline bool cp_integer(const String &str, long *result);
inline bool cp_integer(const String &str, unsigned long *result);
#if HAVE_LONG_LONG
inline bool cp_integer(const String &str, long long *result);
inline bool cp_integer(const String &str, unsigned long long *result);
#elif HAVE_INT64_TYPES && !HAVE_INT64_IS_LONG
inline bool cp_integer(const String &str, int64_t *result);
inline bool cp_integer(const String &str, uint64_t *result);
#endif



#define cp_integer64 cp_integer
#define cp_unsigned64 cp_integer
#define cp_unsigned cp_integer





/** @brief  Parse an integer from [@a begin, @a end) in base @a base.
 * @param  begin  first character in string
 * @param  end    one past last character in string
 * @param  base   base of integer: 0 or 2-36
 * @param[out]  result  stores parsed result
 * @return  pointer to first unparsed character in string; equals @a begin
 *       if the string didn't start with a valid integer
 *
 * This function parses an integer from the initial characters of a string.
 * The resulting integer is stored in *@a result.
 *
 * The integer format consists of an optional initial sign <tt>+/-</tt>,
 * followed by one or more digits.  A negative sign is only accepted if @a
 * result has a signed type.  Digits may be separated by underscores (to make
 * numbers easier to read), but the first and last characters in the integer
 * cannot be underscores, and two underscores can't appear in a row.  Some
 * examples:
 *
 * @code
 * 0
 * 0x100
 * -1_000_023
 * @endcode
 *
 * Digits are numbered from 0-9, then A-Z/a-z.  @a base determines which
 * digits are legal.  If @a base is 0, then a leading <tt>0x</tt> or
 * <tt>0X</tt> may precede the digits, indicating base 16; a leading
 * <tt>0</tt> indicates base 8; anything else is base 10.
 *
 * Returns the first character that can't be parsed as part of the integer.
 * If there is no valid integer at the beginning of the string, then returns
 * @a begin; *@a result is unchanged.
 *
 * This function checks for overflow.  If an integer is too large for @a
 * result, then the maximum possible value is stored in @a result and the
 * cp_errno variable is set to CPE_OVERFLOW.  Otherwise, cp_errno is set to
 * CPE_FORMAT (for no valid integer) or CPE_OK (if all was well).
 *
 * Overloaded versions of this function are available for int, unsigned int,
 * long, unsigned long, and (depending on configuration) long long and
 * unsigned long long @a result values.
 */
inline const char *cp_integer(const char *begin, const char *end, int base, int *result)
{
    return cp_basic_integer(begin, end, base, -(int) sizeof(*result), result);
}

/** @brief  Parse an integer from @a str in base @a base.
 * @param  str   string
 * @param  base  base of integer: 0 or 2-36
 * @param[out]  result  stores parsed result
 * @return  True if @a str parsed correctly, false otherwise.
 *
 * Parses an integer from an input string.  If the string correctly parses as
 * an integer, then the resulting value is stored in *@a result and the
 * function returns true.  Otherwise, *@a result remains unchanged and the
 * function returns false.
 *
 * Overloaded versions are available for int, unsigned int, long, unsigned
 * long, and (depending on configuration) long long and unsigned long long @a
 * result values.
 *
 * @sa cp_integer(const char *, const char *, int, int *) for the rules on
 * parsing integers.
 */
inline bool cp_integer(const String &str, int base, int *result)
{
    return cp_basic_integer(str.begin(), str.end(), base + cp_basic_integer_whole, -(int) sizeof(*result), result) != str.begin();
}


inline const char *cp_integer(const char *begin, const char *end, int base, unsigned *result)
{
    return cp_basic_integer(begin, end, base, (int) sizeof(*result), result);
}

/// @cond never
inline const unsigned char *cp_integer(const unsigned char *begin, const unsigned char *end, int base, unsigned *result)
{
    return reinterpret_cast<const unsigned char *>(cp_integer(reinterpret_cast<const char *>(begin), reinterpret_cast<const char *>(end), base, result));
}
/// @endcond


inline const char *cp_integer(const char *begin, const char *end, int base, long *result)
{
    return cp_basic_integer(begin, end, base, -(int) sizeof(*result), result);
}

inline const char *cp_integer(const char *begin, const char *end, int base, unsigned long *result)
{
    return cp_basic_integer(begin, end, base, (int) sizeof(*result), result);
}

/// @cond never
inline const unsigned char *cp_integer(const unsigned char *begin, const unsigned char *end, int base, unsigned long *result)
{
    return reinterpret_cast<const unsigned char *>(cp_integer(reinterpret_cast<const char *>(begin), reinterpret_cast<const char *>(end), base, result));
}
/// @endcond


#if HAVE_LONG_LONG

inline const char *cp_integer(const char *begin, const char *end, int base, long long *result)
{
    return cp_basic_integer(begin, end, base, -(int) sizeof(*result), result);
}

inline const char *cp_integer(const char *begin, const char *end, int base, unsigned long long *result)
{
    return cp_basic_integer(begin, end, base, (int) sizeof(*result), result);
}

/// @cond never
inline const unsigned char *cp_integer(const unsigned char *begin, const unsigned char *end, int base, unsigned long long *result)
{
    return reinterpret_cast<const unsigned char *>(cp_integer(reinterpret_cast<const char *>(begin), reinterpret_cast<const char *>(end), base, result));
}
/// @endcond

#elif HAVE_INT64_TYPES && !HAVE_INT64_IS_LONG

inline const char *cp_integer(const char *begin, const char *end, int base, int64_t *result)
{
    return cp_basic_integer(begin, end, base, -(int) sizeof(*result), result);
}

inline const char *cp_integer(const char *begin, const char *end, int base, uint64_t *result)
{
    return cp_basic_integer(begin, end, base, (int) sizeof(*result), result);
}

/// @cond never
inline const unsigned char *cp_integer(const unsigned char *begin, const unsigned char *end, int base, uint64_t *result)
{
    return reinterpret_cast<const unsigned char *>(cp_integer(reinterpret_cast<const char *>(begin), reinterpret_cast<const char *>(end), base, result));
}
/// @endcond

#endif

inline bool cp_integer(const String &str, int base, int *result);

inline bool cp_integer(const String &str, int base, unsigned int *result)
{
    return cp_basic_integer(str.begin(), str.end(), base + cp_basic_integer_whole, (int) sizeof(*result), result) != str.begin();
}

inline bool cp_integer(const String &str, int base, long *result)
{
    return cp_basic_integer(str.begin(), str.end(), base + cp_basic_integer_whole, -(int) sizeof(*result), result) != str.begin();
}

inline bool cp_integer(const String &str, int base, unsigned long *result)
{
    return cp_basic_integer(str.begin(), str.end(), base + cp_basic_integer_whole, (int) sizeof(*result), result) != str.begin();
}

#if HAVE_LONG_LONG

inline bool cp_integer(const String &str, int base, long long *result)
{
    return cp_basic_integer(str.begin(), str.end(), base + cp_basic_integer_whole, -(int) sizeof(*result), result) != str.begin();
}

inline bool cp_integer(const String &str, int base, unsigned long long *result)
{
    return cp_basic_integer(str.begin(), str.end(), base + cp_basic_integer_whole, (int) sizeof(*result), result) != str.begin();
}

#elif HAVE_INT64_TYPES && !HAVE_INT64_IS_LONG

inline bool cp_integer(const String &str, int base, int64_t *result)
{
    return cp_basic_integer(str.begin(), str.end(), base + cp_basic_integer_whole, -(int) sizeof(*result), result) != str.begin();
}

inline bool cp_integer(const String &str, int base, uint64_t *result)
{
    return cp_basic_integer(str.begin(), str.end(), base + cp_basic_integer_whole, (int) sizeof(*result), result) != str.begin();
}

#endif

/** @brief  Parse an integer from @a str in base 0.
 *
 *  Same as cp_integer(str, 0, result). */
inline bool cp_integer(const String &str, int *result)
{
    return cp_basic_integer(str.begin(), str.end(), cp_basic_integer_whole, -(int) sizeof(*result), result) != str.begin();
}

inline bool cp_integer(const String &str, unsigned int *result)
{
    return cp_basic_integer(str.begin(), str.end(), cp_basic_integer_whole, (int) sizeof(*result), result) != str.begin();
}

inline bool cp_integer(const String &str, long *result)
{
    return cp_basic_integer(str.begin(), str.end(), cp_basic_integer_whole, -(int) sizeof(*result), result) != str.begin();
}

inline bool cp_integer(const String &str, unsigned long *result)
{
    return cp_basic_integer(str.begin(), str.end(), cp_basic_integer_whole, (int) sizeof(*result), result) != str.begin();
}

#if HAVE_LONG_LONG

inline bool cp_integer(const String &str, long long *result)
{
    return cp_basic_integer(str.begin(), str.end(), cp_basic_integer_whole, -(int) sizeof(*result), result) != str.begin();
}

inline bool cp_integer(const String &str, unsigned long long *result)
{
    return cp_basic_integer(str.begin(), str.end(), cp_basic_integer_whole, (int) sizeof(*result), result) != str.begin();
}

#elif HAVE_INT64_TYPES && !HAVE_INT64_IS_LONG

inline bool cp_integer(const String &str, int64_t *result)
{
    return cp_basic_integer(str.begin(), str.end(), cp_basic_integer_whole, -(int) sizeof(*result), result) != str.begin();
}

inline bool cp_integer(const String &str, uint64_t *result)
{
    return cp_basic_integer(str.begin(), str.end(), cp_basic_integer_whole, (int) sizeof(*result), result) != str.begin();
}

#endif




#endif

