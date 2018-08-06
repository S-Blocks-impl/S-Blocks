#include "../include/config.h"
#define CLICK_COMPILING_CONFPARSE_CC 1
#include "./glue.h"
#include "confparse.h"
#include "args.h"
#include "error.h"
#include "straccum.h"
#include "../include/type_traits.h"
#include <ctype.h> //isspace

//# include <pwd.h>
# include <stdarg.h>

#include "timestamp.h"


int cp_errno;




static const char *
skip_comment(const char *s, const char *end)
{
  assert(s + 1 < end && *s == '/' && (s[1] == '/' || s[1] == '*'));

  if (s[1] == '/') {
    for (s += 2; s + 1 < end && *s != '\n' && *s != '\r'; s++)
      /* nada */;
    if (s + 1 < end && *s == '\r' && s[1] == '\n')
      s++;
    return s + 1;
  } else { /* s[1] == '*' */
    for (s += 2; s + 2 < end && (*s != '*' || s[1] != '/'); s++)
      /* nada */;
    return s + 2;
  }
}

static const char *
skip_backslash(const char *s, const char *end)
{
  assert(s < end && *s == '\\');

  if (s + 1 >= end)
    return s + 1;
  else if (s[1] == '<') {
    for (s += 2; s < end; )
      if (*s == '>')
    return s + 1;
      else if (*s == '/' && s + 1 < end && (s[1] == '/' || s[1] == '*'))
    s = skip_comment(s, end);
      else
    s++;
    return s;
  } else if (s[1] == '\r' && s + 2 < end && s[2] == '\n')
    return s + 3;
  else
    return s + 2;
}

/** @brief  Return the first character after a double-quoted string starting at @a begin.
 *  @param  begin  beginning of double-quoted string
 *  @param  end    one past end of string
 *  @return Pointer to first character in [@a begin, @a end) after the
 *          double-quoted string, or @a end if the double-quoted portion is not
 *          correctly terminated.
 *  @pre    @a begin < @a end and *@a begin == '\"'
 *
 *  cp_skip_double_quote() understands all the backslash escapes processed
 *  by cp_process_backslash(). */
const char *
cp_skip_double_quote(const char *begin, const char *end)
{
  assert(begin < end && *begin == '\"');

  for (begin++; begin < end; )
    if (*begin == '\\')
      begin = skip_backslash(begin, end);
    else if (*begin == '\"')
      return begin + 1;
    else
      begin++;

  return end;
}


static const char *
skip_single_quote(const char *s, const char *end)
{
  assert(s < end && *s == '\'');

  for (s++; s < end; s++)
    if (*s == '\'')
      return s + 1;

  return end;
}

/// @brief  Find the first nonspace, noncomment character in the string [@a begin, @a end).
/// @param  begin  beginning of string
/// @param  end    one past end of string
/// @return Pointer to first nonspace and noncomment character in [@a begin,
///     @a end), or @a end if the string is all spaces and comments.
///
/// This function recognizes C-style and C++-style comments:
/// @code
/// /* C style */  // C++ style (runs until newline)
/// @endcode
/// In C++-style comments, the character
/// sequences <tt>"\n"</tt>, <tt>"\r"</tt>, and <tt>"\r\n"</tt> are
/// recognized as newlines.  The newline is considered part of the comment.
const char *
cp_skip_comment_space(const char *begin, const char *end)
{
  for (; begin < end; begin++) {
    if (isspace((unsigned char) *begin))
      /* nada */;
    else if (*begin == '/' && begin + 1 < end && (begin[1] == '/' || begin[1] == '*'))
      begin = skip_comment(begin, end) - 1;
    else
      break;
  }
  return begin;
}




static String
partial_uncomment(const String &str, int i, int *comma_pos)
{
  const char *s = str.data() + i;
  const char *end = str.end();

  // skip initial spaces
  s = cp_skip_comment_space(s, end);

  // accumulate text, skipping comments
  StringAccum sa;
  const char *left = s;
  const char *right = s;
  bool closed = false;

  while (s < end) {
    if (isspace((unsigned char) *s))
      s++;
    else if (*s == '/' && s + 1 < end && (s[1] == '/' || s[1] == '*')) {
      s = skip_comment(s, end);
      closed = true;
    } else if (*s == ',' && comma_pos)
      break;
    else {
      if (closed) {
    sa << str.substring(left, right) << ' ';
    left = s;
    closed = false;
      }
      if (*s == '\'')
    s = skip_single_quote(s, end);
      else if (*s == '\"')
    s = cp_skip_double_quote(s, end);
      else if (*s == '\\' && s + 1 < end && s[1] == '<')
    s = skip_backslash(s, end);
      else
    s++;
      right = s;
    }
  }

  if (comma_pos)
    *comma_pos = s - str.begin();
  if (!sa)
    return str.substring(left, right);
  else {
    sa << str.substring(left, right);
    return sa.take_string();
  }
}


/// @brief  Process a backslash escape, appending results to @a sa.
/// @param  begin  beginning of string
/// @param  end    end of string
/// @param  sa     string accumulator
/// @pre  @a begin < @a end, and @a begin points to a backslash character.
/// @return A pointer to the first character in [@a begin, @a end) following
///     the backslash escape.
///
/// This function understands the following backslash escapes.
/// <ul>
/// <li><tt>"\[newline]"</tt> is ignored (it adds no characters to @a sa),
/// where <tt>[newline]</tt> is one of the sequences <tt>"\n"</tt>,
/// <tt>"\r"</tt>, or <tt>"\r\n"</tt>.</li>
/// <li><tt>"\[C escape]"</tt> is processed as in C, where <tt>[C escape]</tt>
/// is one of the characters in <tt>[abfnrtv]</tt>.</li>
/// <li><tt>"\\"</tt> expands to a single backslash.  Similarly,
/// <tt>"\$"</tt>, <tt>"\'"</tt>, <tt>"\\""</tt>, and <tt>"\,"</tt>
/// expand to the escaped character.</li>
/// <li><tt>"\[1-3 octal digits]"</tt> expands to the given character.</li>
/// <li><tt>"\x[hex digits]"</tt> expands to the given character.</li>
/// <li><tt>"\<[hex digits, spaces, and comments]>"</tt> expands to the
/// binary string indicated by the <tt>hex digits</tt>.  Spaces and comments
/// are removed.  For example,
/// @code
/// "\<48656c6C 6f 2 /* And finally */ 1>" expands to "Hello!"
/// @endcode
/// (This example should begin with <tt>"\<"</tt>; it may not because of Doxygen problems.)</li>
/// <li>A backslash at the end of the string expands to a backslash.</li>
/// </ul>
const char *
cp_process_backslash(const char *begin, const char *end, StringAccum &sa)
{
  assert(begin < end && *begin == '\\');

  if (begin == end - 1) {
    sa << '\\';
    return end;
  }

  switch (begin[1]) {

   case '\r':
    return (begin + 2 < end && begin[2] == '\n' ? begin + 3 : begin + 2);

   case '\n':
    return begin + 2;

   case 'a': sa << '\a'; return begin + 2;
   case 'b': sa << '\b'; return begin + 2;
   case 'f': sa << '\f'; return begin + 2;
   case 'n': sa << '\n'; return begin + 2;
   case 'r': sa << '\r'; return begin + 2;
   case 't': sa << '\t'; return begin + 2;
   case 'v': sa << '\v'; return begin + 2;

   case '0': case '1': case '2': case '3':
   case '4': case '5': case '6': case '7': {
     int c = 0, d = 0;
     for (begin++; begin < end && *begin >= '0' && *begin <= '7' && d < 3;
    begin++, d++)
       c = c*8 + *begin - '0';
     sa << (char)c;
     return begin;
   }

   case 'x': {
     int c = 0;
     for (begin += 2; begin < end; begin++)
       if (*begin >= '0' && *begin <= '9')
   c = c*16 + *begin - '0';
       else if (*begin >= 'A' && *begin <= 'F')
   c = c*16 + *begin - 'A' + 10;
       else if (*begin >= 'a' && *begin <= 'f')
   c = c*16 + *begin - 'a' + 10;
       else
   break;
     sa << (char)c;
     return begin;
   }

   case '<': {
     int c = 0, d = 0;
     for (begin += 2; begin < end; begin++) {
       if (*begin == '>')
   return begin + 1;
       else if (*begin >= '0' && *begin <= '9')
   c = c*16 + *begin - '0';
       else if (*begin >= 'A' && *begin <= 'F')
   c = c*16 + *begin - 'A' + 10;
       else if (*begin >= 'a' && *begin <= 'f')
   c = c*16 + *begin - 'a' + 10;
       else if (*begin == '/' && begin + 1 < end && (begin[1] == '/' || begin[1] == '*')) {
   begin = skip_comment(begin, end) - 1;
   continue;
       } else
   continue;  // space (ignore it) or random (error)
       if (++d == 2) {
   sa << (char)c;
   c = d = 0;
       }
     }
     // ran out of space in string
     return end;
   }

   case '\\': case '\'': case '\"': case '$': case ',':
   default:
    sa << begin[1];
    return begin + 2;

  }
}



/// @brief  Remove one level of quoting from @a str, returning the result.
///
/// This function acts as cp_uncomment, plus removing one level of quoting.
/// <tt>"..."</tt> and <tt>'...'</tt> sequences are replaced by their contents.
/// Backslash escapes are expanded inside double quotes (see
/// cp_process_backslash).  Additionally, <tt>"\<...>"</tt> sequences are
/// expanded outside of any quotes.  For example:
/// @code
/// cp_unquote("\"\\n\" abc /* 123 */ '/* def */'") == "\n abc /* def */"
/// @endcode
String
cp_unquote(const String &str)
{
  String xtr = partial_uncomment(str, 0, 0);
  const char *s = xtr.data();
  const char *end = xtr.end();

  // accumulate a word
  StringAccum sa;
  const char *start = s;
  int quote_state = 0;

  for (; s < end; s++)
    switch (*s) {

     case '\"':
     case '\'':
      if (quote_state == 0) {
    sa << xtr.substring(start, s); // null string if start >= s
    start = s + 1;
    quote_state = *s;
      } else if (quote_state == *s) {
    sa << xtr.substring(start, s);
    start = s + 1;
    quote_state = 0;
      }
      break;

     case '\\':
      if (s + 1 < end && (quote_state == '\"'
              || (quote_state == 0 && s[1] == '<'))) {
    sa << xtr.substring(start, s);
    start = cp_process_backslash(s, end, sa);
    s = start - 1;
      }
      break;

    }

  if (start == xtr.begin())
    return xtr;
  else {
    sa << xtr.substring(start, s);
    return sa.take_string();
  }
}







/// @brief  Separate a configuration string into arguments at commas.
/// @param       str   configuration string
/// @param[out]  conf  arguments
///
/// The configuration string is broken into arguments at unquoted commas.
/// Each argument is passed through cp_uncomment(), then appended to @a conf.
/// If the final argument is empty, it is ignored.  For example:
/// @code
/// cp_argvec("a, b, c", v)            appends  "a", "b", "c"
/// cp_argvec("  a /*?*/ b,  c, ", v)  appends  "a b", "c"
/// cp_argvec("\"x, y\" // ?", v)      appends  "\"x, y\""
/// @endcode
void
cp_argvec(const String &str, stlpmtx_std::vector<String> &conf)
{
  // common case: no configuration
  int len = str.length();
  if (len == 0)
    return;

  for (int pos = 0; pos < len; pos++) {
    String arg = partial_uncomment(str, pos, &pos);
    // add the argument if it is nonempty or not the last argument
    if (arg || pos < len)
      conf.push_back(arg);
  }
}



static const char *
skip_spacevec_item(const char *s, const char *end)
{
  while (s < end)
    switch (*s) {

     case '/':
      // a comment ends the item
      if (s + 1 < end && (s[1] == '/' || s[1] == '*'))
    return s;
      s++;
      break;

     case '\"':
      s = cp_skip_double_quote(s, end);
      break;

     case '\'':
      s = skip_single_quote(s, end);
      break;

     case '\\':         // check for \<...> strings
      if (s + 1 < end && s[1] == '<')
    s = skip_backslash(s, end);
      else
    s++;
      break;

     case ' ':
     case '\f':
     case '\n':
     case '\r':
     case '\t':
     case '\v':
      return s;

     default:
      s++;
      break;

    }
  return s;
}



/// @brief  Separate a configuration string into arguments at unquoted spaces.
/// @param       str   configuration string
/// @param[out]  conf  arguments
///
/// The configuration string is broken into arguments at unquoted spaces.
/// Each argument is passed through cp_uncomment(), then appended to @a conf.
/// If the final argument is empty, it is ignored.  For example:
/// @code
/// cp_spacevec("a  b, c", v)            appends  "a", "b,", "c"
/// cp_spacevec("  'a /*?*/ b'c", v)     appends  "'a /*?*/ b'c"
/// @endcode
void
cp_spacevec(const String &str, stlpmtx_std::vector<String> &conf)
{
  // common case: no configuration
  if (str.length() == 0)
    return;

  // collect arguments like cp_shift_spacevec
  const char *s = str.data();
  const char *end = str.end();
  while ((s = cp_skip_comment_space(s, end)) < end) {
    const char *t = skip_spacevec_item(s, end);
    conf.push_back(str.substring(s, t));
    s = t;
  }
}


String
cp_shift_spacevec(String &str)
{
  const char *item = cp_skip_comment_space(str.begin(), str.end());
  const char *item_end = skip_spacevec_item(item, str.end());
  String answer = str.substring(item, item_end);
  item_end = cp_skip_comment_space(item_end, str.end());
  str = str.substring(item_end, str.end());
  return answer;
}







// PARSING STRINGS

/** @brief Parse a string from @a str.
 * @param  str  string
 * @param[out]  result  stores parsed result
 * @param[out]  rest  (optional) stores unparsed portion of @a str
 * @return  True if @a str parsed correctly, false otherwise.
 *
 * Parses a string from @a str.  The first unquoted space encountered ends the
 * string, but spaces are allowed within single or double quotes.  Unquoted
 * empty strings are not accepted.  If the string fully parses, then the
 * result is unquoted by cp_unquote() and stored in *@a result and the function
 * returns true.  Otherwise, *@a result remains unchanged and the function
 * returns false.
 *
 * If @a rest is nonnull, then the string doesn't need to fully parse; the
 * part of the string starting with the first unquoted space is stored in *@a
 * rest and the function returns true.
 */
bool
cp_string(const String &str, String *result, String *rest)
{
  const char *s = str.data();
  const char *end = str.end();

  // accumulate a word
  while (s < end)
    switch (*s) {

     case ' ':
     case '\f':
     case '\n':
     case '\r':
     case '\t':
     case '\v':
      goto done;

     case '\"':
      s = cp_skip_double_quote(s, end);
      break;

     case '\'':
      s = skip_single_quote(s, end);
      break;

     case '\\':
      if (s + 1 < end && s[1] == '<')
    s = skip_backslash(s, end);
      else
    s++;
      break;

     default:
      s++;
      break;

    }

 done:
  if (s == str.begin() || (!rest && s != end))
    return false;
  else {
    if (rest)
      *rest = str.substring(s, end);
    *result = cp_unquote(str.substring(str.begin(), s));
    return true;
  }
}







// PARSING REAL NUMBERS

static uint32_t exp10val[] = { 1, 10, 100, 1000, 10000, 100000, 1000000,
                   10000000, 100000000, 1000000000 };

bool
cp_real10(const String &str, int frac_digits, int exponent_delta,
      uint32_t *return_int_part, uint32_t *return_frac_part)
{
    DecimalFixedPointArg dfpa(frac_digits, exponent_delta);
    if (!dfpa.parse_saturating(str, *return_int_part, *return_frac_part)) {
    cp_errno = CPE_FORMAT;
    return false;
    } else if (dfpa.status == NumArg::status_range)
    cp_errno = CPE_OVERFLOW;
    else
    cp_errno = CPE_OK;
    return true;
}

/** @brief Parse a real number from @a str, representing the result as an
 * integer with @a frac_digits decimal digits of fraction.
 * @param  str  string
 * @param  frac_digits  number of decimal digits of fraction, 0-9
 * @param[out]  result_int_part  stores integer portion of parsed result
 * @param[out]  result_frac_part  stores fractional portion of parsed result
 * @return  True if @a str parsed correctly, false otherwise.
 *
 * Parses an unsigned real number from an input string.  The result is
 * represented with @a frac_digits decimal digits of fraction.  The integer
 * and fraction parts of the result are stored in two separate integers, @a
 * result_int_part and @a result_frac_part.  For example, the number 10.5
 * would be represented as 10 and 5 if @a frac_digits == 1, or 10 and 5000 if
 * @a frac_digits == 4.  If the string fully parses, then the resulting value
 * is stored in the result variables and the function returns true.
 * Otherwise, the result variables remains unchanged and the function returns
 * false.
 *
 * The real number format and error conditions are the same as for cp_real2().
 * (Negative numbers are not allowed.)
 */
bool
cp_real10(const String &str, int frac_digits,
      uint32_t *result_int_part, uint32_t *result_frac_part)
{
    return cp_real10(str, frac_digits, 0, result_int_part, result_frac_part);
}

bool
cp_real10(const String &str, int frac_digits, int exponent_delta,
      uint32_t *result)
{
    DecimalFixedPointArg dfpa(frac_digits, exponent_delta);
    if (!dfpa.parse_saturating(str, *result)) {
    cp_errno = CPE_FORMAT;
    return false;
    } else if (dfpa.status == dfpa.status_range)
    cp_errno = CPE_OVERFLOW;
    else
    cp_errno = CPE_OK;
    return true;
}

bool
cp_real10(const String &str, int frac_digits, uint32_t *result)
{
    return cp_real10(str, frac_digits, 0, result);
}














const char *
cp_basic_integer(const char *begin, const char *end, int flags, int size,
         void *result)
{
    IntArg ia(flags & 63);
    IntArg::limb_type limbs[4];
    int usize = size < 0 ? -size : size;
    const char *x = ia.parse(begin, end,
                 size < 0, usize,
                 limbs, usize / sizeof(IntArg::limb_type));
    if ((ia.status && ia.status != IntArg::status_range)
    || ((flags & cp_basic_integer_whole) && x != end)) {
    cp_errno = CPE_FORMAT;
    return begin;
    } else if (ia.status == IntArg::status_ok)
    cp_errno = CPE_OK;
    else
    cp_errno = CPE_OVERFLOW;

    // assign
    if (usize == 1)
    extract_integer(limbs, *static_cast<unsigned char *>(result));
    else if (usize == sizeof(short))
    extract_integer(limbs, *static_cast<unsigned short *>(result));
    else if (usize == sizeof(int))
    extract_integer(limbs, *static_cast<unsigned *>(result));
    else if (usize == sizeof(long))
    extract_integer(limbs, *static_cast<unsigned long *>(result));
#if HAVE_LONG_LONG
    else if (usize == sizeof(long long))
    extract_integer(limbs, *static_cast<unsigned long long *>(result));
#endif
#if HAVE_INT64_TYPES && (!HAVE_LONG_LONG || (!HAVE_INT64_IS_LONG && !HAVE_INT64_IS_LONG_LONG))
    else if (usize == sizeof(int64_t))
    extract_integer(limbs, *static_cast<uint64_t *>(result));
#endif
    else
    assert(0);

    return x;
}



















// PARSING TIME

static const char *
read_unit(const char *s, const char *end,
      const char *unit_begin_in, int unit_len, const char *prefix,
      int *power, int *factor)
{
  const char *work = end;
  const unsigned char *unit_begin = reinterpret_cast<const unsigned char *>(unit_begin_in);
  const unsigned char *unit = unit_begin + unit_len;
  if (unit > unit_begin && unit[-1] == 0)
    unit--;
  while (unit > unit_begin) {
    if (unit[-1] < 4) {
    int type = unit[-1];
    assert(unit - 3 - (type >= 2) >= unit_begin);
    *factor = unit[-2];
    if (type >= 2)
        unit--, *factor += 256 * unit[-2];
    *power = (type & 1 ? -(int) unit[-3] : unit[-3]);

    // check for SI prefix
    if (prefix && work > s) {
        for (; *prefix; prefix += 2)
        if (*prefix == work[-1]) {
            *power += (int) prefix[1] - 64;
            work--;
            break;
        }
    }

    while (work > s && isspace((unsigned char) work[-1]))
        work--;
    return work;
    } else if (unit[-1] != (unsigned char) work[-1]) {
      while (unit > unit_begin && unit[-1] >= 4)
    unit--;
      unit -= 3 + (unit[-1] >= 2);
      work = end;
    } else {
      unit--;
      work--;
    }
  }
  return end;
}

static const char seconds_units[] = "\
\0\1\0s\
\0\1\0sec\
\1\6\0m\
\1\6\0min\
\2\044\0h\
\2\044\0hr\
\2\003\140\2d\
\2\003\140\2day";
static const char seconds_prefixes[] = "m\075u\072n\067";

/** @brief Parse a timestamp from @a str.
 * @param  str  string
 * @param[out]  result  stores parsed result
 * @param  allow_negative  allow negative timestamps if true
 * @return  True if @a str parsed correctly, false otherwise.
 *
 * Parses a timestamp from @a str.  Timestamps are expressed as fractional
 * amounts of seconds, usually measured in Unix time, such as
 * <tt>"1189383079.180265331"</tt>.  The input format accepts the unit
 * suffixes described at cp_seconds_as.  If the string fully parses, then the
 * resulting value is stored in *@a result and the function returns true.
 * Otherwise, *@a result remains unchanged and the function returns false.
 *
 * If a number is too large for @a result, then the maximum possible value is
 * stored in @a result and the cp_errno variable is set to CPE_OVERFLOW;
 * otherwise, cp_errno is set to CPE_FORMAT (unparsable) or CPE_OK (if all was
 * well).
 *
 * An overloaded version of this function is available for struct timeval @a
 * result values.
 */
bool cp_time(const String &str, Timestamp *result, bool allow_negative)
{
    int power = 0, factor = 1;
    const char *begin = str.begin(), *end = str.end();
    const char *after_unit = read_unit(begin, end, seconds_units, sizeof(seconds_units), seconds_prefixes, &power, &factor);

    bool negative = false;
    if (allow_negative && after_unit - begin > 1
    && begin[0] == '-' && begin[1] != '+') {
    negative = true;
    ++begin;
    }

    uint32_t sec, nsec;
    if (!cp_real10(str.substring(begin, after_unit), 9, power, &sec, &nsec))
    return false;
    if (factor != 1) {
    nsec *= factor;
    int delta = nsec / 1000000000;
    nsec -= delta * 1000000000;
    sec = (sec * factor) + delta;
    }
    if (!negative)
    *result = Timestamp::make_nsec(sec, nsec);
    else
    *result = Timestamp::make_nsec(-(Timestamp::seconds_type) sec, nsec);
    return true;
}

bool cp_time(const String &str, timeval *result)
{
#if TIMESTAMP_PUNS_TIMEVAL
    return cp_time(str, reinterpret_cast<Timestamp*>(result));
#else
    Timestamp t;
    if (cp_time(str, &t)) {
    *result = t.timeval();
    return true;
    } else
    return false;
#endif
}
