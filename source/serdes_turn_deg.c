/** Some functions to serialize/deserialize an angle @ turn expressed by a fixed point number to/from degree.
    \file serdes_turn_deg.c
    \author OOTA, Masato
    \copyright Copyright © 2022 OOTA, Masato
    \par License GPL-3.0-or-later
    \parblock
      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 3 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program.  If not, see <http://www.gnu.org/licenses/>.
    \endparblock

    \par Legend
    \parblock
      \verbatim
                Precision#   -2  -1   0   1   2   3   4   5
           Place of Digit#    2   1   0  -1  -2  -3  -4  -5
      Index of Digit Array    0   1   2   3   4   5   6   7
                            +---+---+---+---+---+---+---+---+
                            | 3 | 5 | 9 | 9 | 9 | 9 | 9 | 1 |
                            +---+---+---+---+---+---+---+---+
                                        ^
                                 Decimal Point
      \endverbatim
    \endparblock
*/
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>

#include "serdes_turn_deg.h"

#if UINT_MAX < 0xFFFFFFFFu
#error UINT_MAX is too small.
#endif

/** The highest place of digit in the serialized string. */
#define SERDES_TURN_DEG_MAX_PLACE (-SERDES_TURN_DEG_MIN_PRECISION)
/** The lowest place of digit in the serialized string. */
#define SERDES_TURN_DEG_MIN_PLACE (-SERDES_TURN_DEG_MAX_PRECISION)

#if SERDES_TURN_DEG_BUF_SIZE < SERDES_TURN_DEG_MAX_PLACE - SERDES_TURN_DEG_MIN_PLACE + 1 + 2 /* digits, decimal point, and NUL */
#error SERDES_TURN_DEG_BUF_SIZE is too small.
#endif

/** The index that is equivalent the place\#k. */
#define BUF_INDEX(k) (SERDES_TURN_DEG_MAX_PLACE - (k))

/** Make sure the parameter is in range.
    \param [inout] param The parameter to check.
    \param [in] minimum The minimum acceptable value.
    \param [in] maximum The maximum acceptable value.

    An assertion error is raised if param is out of range.

    If NDEBUG is defined, param is clamped to the range.
*/
#define PARAM_CHECK(param, minimum, maximum) \
    do { \
        assert((minimum) <= param && param <= (maximum)); \
        param = param > (maximum) ? (maximum) : param; \
        param = param < (minimum) ? (minimum) : param; \
    } while (0)


/** Convert an angle @ turn to the digit array @ degree.
    \param [inout] buf A buffer to output a digit array. The length must be at least SERDES_TURN_DEG_BUF_SIZE - 2.
    \param [in] turn The angle @ turn expressed by a fixed point of the 0 bit integer part and bit_width bit decimal place. It means the angle as an integer is in range [0, 2**bit_width - 1], that is [0, 1) turn = [0, 360) degree.
    \param [in] bit_width The bit-width of turn. It must be in range [SERDES_TURN_DEG_MIN_BIT_WIDTH, SERDES_TURN_DEG_MAX_BIT_WIDTH].
    \param [in] place The maximum place of digit to output. It must be in range [SERDES_TURN_DEG_MIN_PLACE, SERDES_TURN_DEG_MAX_PLACE].
    \return The minimum place of digit to output.
    \warning The return value is longer than the specified place if it's not enough to deserialize to the same angle.
    \warning The lowest place in the return value may be 10 if place is less than SERDES_TURN_DEG_MAX_PLACE.

    This function uses a variation of (FPP)2 in dragon4 (https://dl.acm.org/doi/10.1145/93548.93559) to convert an angle @ turn into a digit array @ degree.
 */
static int conv_turn_to_deg_digit_array(char *buf, unsigned int turn, unsigned int bit_width, int place)
{
    unsigned int R;  /* Remain? */
    unsigned int S;  /* The place of interest in R */
    unsigned int M;  /* Margin? */
    int low; /* matched low side */
    int high; /* matched high side */
    int k; /* the place of digit */

    PARAM_CHECK(place, SERDES_TURN_DEG_MIN_PLACE, SERDES_TURN_DEG_MAX_PLACE);
    PARAM_CHECK(bit_width, SERDES_TURN_DEG_MIN_BIT_WIDTH, SERDES_TURN_DEG_MAX_BIT_WIDTH);

    turn &= (1u << bit_width) - 1; /* clamp turn within [0, 360) degree */
    R = turn * 360; /* convert turn into degree */
    S = (1u << bit_width) * 100; /* 100 degree */
    M = 180; /* equivalent to LSB/2 in turn */

    k = SERDES_TURN_DEG_MAX_PLACE + 1;
    low = 0;
    high = 0;
    while (!low && !high) {
        unsigned int U = R / S; /* candidate for the digit */
        k--;
        R = R - U * S;
        if (k <= place) {
            low = R < M;
            high = R + M > S; /* R > S - M can be overflow */
            if (high && (!low || R >= S / 2)) {
                /* if k < 2 && k == place && high, U may be 10. */
                U++;
            }
        }
        buf[BUF_INDEX(k)] = U;
        R *= 10; /* The maximum R may be 999.991 * (1u << bit_width), so SERDES_TURN_DEG_MAX_BIT_WIDTH is 32 - 10. */
        M *= 10;

        assert(k >= SERDES_TURN_DEG_MIN_PLACE);
        if (k == SERDES_TURN_DEG_MIN_PLACE) {
            assert(low || high);
            break;
        }
    }
    return k;
}


/** Carry up an overflow from the lowest place.
    \param [inout] digit_array A digit array @ degree.
    \param [in] lowest_place The lowest place of digit_array.
    \pre digit_array[BUF_INDEX(lowest_place)] may be 10, and the other items in digit_array are not 10.
    \pre digit_array is less than 360.
    \post digit_array doesn't contain 10.
    \post digit_array is less than 360.
*/
static void carry_up_overflow(char *digit_array, const int lowest_place)
{
    int k;
    for (k = lowest_place; k < SERDES_TURN_DEG_MAX_PLACE; k++) {
        if (digit_array[BUF_INDEX(k)] == 10) {
            digit_array[BUF_INDEX(k)] = 0;
            digit_array[BUF_INDEX(k + 1)]++;
        } else {
            break;
        }
    }
#ifndef NDEBUG
    if (lowest_place < 2) {
        /* The result must not be more than or equal to 360. */
        assert(!(digit_array[0] == 3 && digit_array[1] == 6));
    }
#endif
}


/** Convert a digit array @ degree to the string.
    \param [inout] dest A buffer to output a string. The length must be at least SERDES_TURN_DEG_BUF_SIZE.
    \param [in] src A digit array to convert.
    \param [in] lowest_place The lowest place of src.
    \return The point to NUL character in buf.
    \note src can overlap the area beyond dest + 1 (including dest + 1).
    \verbatim
     Digit Array#    0   1   2   3   4   5   6   7
                   +---+---+---+---+---+---+---+---+
         src       | 3 | 5 | 9 | 9 | 9 | 9 | 9 | 1 |
                   +---+---+---+---+---+---+---+---+
                    /   /   /    |   |   |   |   |
                   /   /   /     |   |   |   |   |
                  /   /   /      |   |   |   |   |
Index of dest    0   1   2   3   4   5   6   7   8   9
               +---+---+---+---+---+---+---+---+---+---+
         dest  |'3'|'5'|'9'|'.'|'9'|'9'|'9'|'9'|'1'|NUL|
               +---+---+---+---+---+---+---+---+---+---+
    \endverbatim
*/
static char *conv_deg_digit_array_to_string(char *dest, const char *src, int lowest_place)
{
#ifndef NDEBUG
    const char *const src_begin = src;
#endif
    int k = SERDES_TURN_DEG_MAX_PLACE;
    char *const dest_begin = dest;

    PARAM_CHECK(lowest_place, SERDES_TURN_DEG_MIN_PLACE, SERDES_TURN_DEG_MAX_PLACE);

    for (; k >= lowest_place; k--) {
        if (k > 0 && *src == 0 && dest == dest_begin) {
            /* Zero suppress */
            src++;
            continue;
        }
        if (k == -1) {
            *dest++ = '.';
        }
        *dest++ = '0' + *src++;
    }
    if (dest == dest_begin) {
        /* This condition can be true if lowest_place > 0. */
        *dest++ = '0';
    } else {
        /* The integer part needs to be filled with '0'. */
        for (; k >= 0; k--) {
            *dest++ = '0';
        }
    }
    *dest = '\0';
    assert(src - src_begin < SERDES_TURN_DEG_BUF_SIZE);
    assert(dest - dest_begin < SERDES_TURN_DEG_BUF_SIZE);
    return dest;
}


/* external functions */

char *serialize_turn_to_deg(char *buf, const unsigned int turn, const unsigned int bit_width)
{
    char *const digit_array = buf + 1;
    const int lowest_place = conv_turn_to_deg_digit_array(digit_array, turn, bit_width, SERDES_TURN_DEG_MAX_PLACE);
    /* no need to carry up. */
    return conv_deg_digit_array_to_string(buf, digit_array, lowest_place);
}


char *serialize_turn_to_deg_p(char *buf, const unsigned int turn, const unsigned int bit_width, const int precision)
{
    char *const digit_array = buf + 1;
    const int lowest_place = conv_turn_to_deg_digit_array(digit_array, turn, bit_width, -precision);
    carry_up_overflow(digit_array, lowest_place);
    return conv_deg_digit_array_to_string(buf, digit_array, lowest_place);
}


char *serialize_turn_to_deg_ps(char *buf, const unsigned int turn, const unsigned int bit_width, const int precision)
{
    char *const digit_array = buf + 1;
    int lowest_place = conv_turn_to_deg_digit_array(digit_array, turn, bit_width, -precision);
    carry_up_overflow(digit_array, lowest_place);

    /* suppress lower zero */
    while (lowest_place < 0) {
        if (digit_array[BUF_INDEX(lowest_place)] == 0) {
            lowest_place++;
        } else {
            break;
        }
    }
    return conv_deg_digit_array_to_string(buf, digit_array, lowest_place);
}


unsigned int deserialize_turn_from_deg(const char *const serialized_deg, const unsigned int bit_width, const char **const endptr)
{
    unsigned int deg = 0;
    unsigned int turn;
    unsigned int S;
    int count = 0;
    /* parse serialized_deg */
    const char *ptr = serialized_deg;
    for (; *ptr != '\0'; ptr++) {
        if (!isspace((unsigned char)*ptr)) {
            break;
        }
    }
    for (; *ptr != '\0' && isdigit((unsigned char)*ptr) && count <= SERDES_TURN_DEG_MAX_PLACE; ptr++) {
        deg *= 10;
        deg += *ptr - '0';
        count++;
    }
    deg %= 360;

    S = 1;
    if (*ptr == '.') {
        ptr++;
        count = 0;
        for (; *ptr != '\0' && isdigit((unsigned char)*ptr); ptr++) {
            if (count < -SERDES_TURN_DEG_MIN_PLACE) {
                S *= 10;
                deg *= 10;
                deg += *ptr - '0';
                count++;
            }
        }
    }
    if (endptr != NULL) {
        *endptr = ptr;
    }

    /* convert an angle @ degree to @ turn. */

    /* 100000 == 10**(-SERDES_TURN_MIN_PLACE) */
    assert(S <= 100000);
    S *= 180;
#if !defined(SERDES_TURN_DEG_DEBUG_FORCE_32BIT) && (ULONG_MAX >> SERDES_TURN_DEG_MAX_BIT_WIDTH) >= (100000 * 360) - 1
    {
        unsigned long R = deg;
        R *= 1u << bit_width;
        turn = R / S;
    }
#else
    /* A variation of (FPP)2 in dragon4 (https://dl.acm.org/doi/10.1145/93548.93559). */
    {
        unsigned int R = deg;
        unsigned int i;
        turn = 0;
        for (i = 0; i <= bit_width; i++) {
            const unsigned int U = R / S;
            R -= U * S;
            R <<= 1;
            turn <<= 1;
            turn |= U;
        }
    }
#endif
    if (turn & 1) {
        turn = (turn / 2) + 1;
    } else {
        turn = turn / 2;
    }
    turn &= (1u << bit_width) - 1;
    return turn;
}


unsigned int conv_deg_to_turn(double deg, unsigned int bit_width)
{
    unsigned int turn;

    PARAM_CHECK(bit_width, SERDES_TURN_DEG_MIN_BIT_WIDTH, SERDES_TURN_DEG_MAX_BIT_WIDTH);

    deg = fmod(deg, 360.0);
    deg *= 1u << bit_width;
    turn = (int)deg / 180;
    if (turn & 1) {
        turn = (turn / 2) + 1;
    } else {
        turn = turn / 2;
    }
    return turn;
}
