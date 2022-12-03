/** Some functions to serialize/deserialize an angle @ turn expressed by a fixed point number to/from degree.
    \file serdes_turn_deg.h
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
*/
#ifndef SERDES_TURN_DEG_H_
#define SERDES_TURN_DEG_H_

/** The required buffer length. */
#define SERDES_TURN_DEG_BUF_SIZE 10

/** The maximum precision. */
#define SERDES_TURN_DEG_MAX_PRECISION 5
/** The minimum precision. */
#define SERDES_TURN_DEG_MIN_PRECISION (-2)

/** The maximum bit-width of the turn. */
#define SERDES_TURN_DEG_MAX_BIT_WIDTH 22
/** The minimum bit-width of the turn. */
#define SERDES_TURN_DEG_MIN_BIT_WIDTH 1

/** Serialize an angle @ turn expressed by a fixed point number to @ degree.
    \param [inout] buf A buffer to write a serialized string. The length must be at least SERDES_TURN_DEG_BUF_SIZE.
    \param [in] turn The angle @ turn expressed by a fixed point of the 0 bit integer part and bit_width bit decimal place. It means the angle as an integer is in range [0, 2**bit_width - 1], that is [0, 1) turn = [0, 360) degree.
    \param [in] bit_width The bit-width of turn. It must be in range [SERDES_TURN_DEG_MIN_BIT_WIDTH, SERDES_TURN_DEG_MAX_BIT_WIDTH].
    \return The point to NUL character in buf.
    \invariant turn == deserialize_turn_from_deg(serialize_turn_to_deg_ps(buf, turn, bit_width, precision), bit_width)

    This function serializes an angle @ turn to a shortest string that can recover the same angle. The string expresses a radix 10 number @ degree.
*/
extern char *serialize_turn_to_deg(char *buf, unsigned int turn, unsigned int bit_width);

/** The variation of serialize_turn_to_deg(), which you can specify a minimum digit number.
    \param [inout] buf A buffer to write a serialized string. The length must be at least SERDES_TURN_DEG_BUF_SIZE.
    \param [in] turn The angle @ turn expressed by a fixed point of the 0 bit integer part and bit_width bit decimal place. It means the angle as an integer is in range [0, 2**bit_width - 1], that is [0, 1) turn = [0, 360) degree.
    \param [in] bit_width The bit-width of turn. It must be in range [SERDES_TURN_DEG_MIN_BIT_WIDTH, SERDES_TURN_DEG_MAX_BIT_WIDTH].
    \param [in] precision The minimum digit number after the decimal point. It must be in range [SERDES_TURN_DEG_MIN_PRECISION, SERDES_TURN_DEG_MAX_PRECISION].
    \return The point to NUL character in buf.
    \warning The serialized string is longer than the specified precision if it's not enough to deserialize to the same angle.
    \invariant turn == deserialize_turn_from_deg(serialize_turn_to_deg_ps(buf, turn, bit_width, precision), bit_width)

    The serialized string is a shortest string that can recover the same angle, but the digit number after the decimal point is more than or equal to precision if precision is positive. If it's zero or negative, it specifies the place before the decimal point.
\verbatim
Precision#   -2  -1   0   1   2   3   4   5
            +---+---+---+---+---+---+---+---+
            | 3 | 5 | 9 | 9 | 9 | 9 | 9 | 1 |
            +---+---+---+---+---+---+---+---+
                        ^
                 Decimal Point
\endverbatim
*/
extern char *serialize_turn_to_deg_p(char *buf, unsigned int turn, unsigned int bit_width, int precision);

/** The variation of serialize_turn_to_deg_p(), which suppresses a series of the lowest side 0 after the decimal point.
    \param [inout] buf A buffer to write a serialized string. The length must be at least SERDES_TURN_DEG_BUF_SIZE.
    \param [in] turn The angle @ turn expressed by a fixed point of the 0 bit integer part and bit_width bit decimal place. It means the angle as an integer is in range [0, 2**bit_width - 1], that is [0, 1) turn = [0, 360) degree.
    \param [in] bit_width The bit-width of turn. It must be in range [SERDES_TURN_DEG_MIN_BIT_WIDTH, SERDES_TURN_DEG_MAX_BIT_WIDTH].
    \param [in] precision The minimum digit number after the decimal point. It must be in range [SERDES_TURN_DEG_MIN_PRECISION, SERDES_TURN_DEG_MAX_PRECISION].
    \return The point to NUL character in buf.
    \warning The serialized string is longer than the specified precision if it's not enough to deserialize to the same angle.
    \invariant turn == deserialize_turn_from_deg(serialize_turn_to_deg_ps(buf, turn, bit_width, precision), bit_width)
*/
extern char *serialize_turn_to_deg_ps(char *buf, unsigned int turn, unsigned int bit_width, int precision);

/** Deserialize from degree to an angle @ turn expressed by a fixed point number.
    \param [in] serialized_deg A real number of a degree.
    \param [in] bit_width The bit-width of the return value. It must be in range [SERDES_TURN_DEG_MIN_BIT_WIDTH, SERDES_TURN_DEG_MAX_BIT_WIDTH].
    \param [out] endptr The pointer to set the next character of the last one that used the conversion if endptr is not NULL.
    \return The angle @ turn expressed by a fixed point of the 0 bit integer part and bit_width bit decimal place. It means the angle as an integer is in range [0, 2**bit_width - 1], that is [0, 1) turn = [0, 360) degree.
    \note This function accepts a text matched at the regexp "\s*[0-9]{0,2}(\.[0-9]*)?" as serialized_deg. If *endptr points to a digit, it means the integer part is too long.
*/
extern unsigned int deserialize_turn_from_deg(const char *serialized_deg, unsigned int bit_width, const char **endptr);

/** Convert degree to an angle @ turn expressed by a fixed point number.
    \param [in] deg A real number of a degree.
    \param [in] bit_width The bit-width of the return value. It must be in range [SERDES_TURN_DEG_MIN_BIT_WIDTH, SERDES_TURN_DEG_MAX_BIT_WIDTH].
    \return The angle @ turn expressed by a fixed point of the 0 bit integer part and bit_width bit decimal place. It means the angle as an integer is in range [0, 2**bit_width - 1], that is [0, 1) turn = [0, 360) degree.
*/
extern unsigned int conv_deg_to_turn(double deg, unsigned int bit_width);

#endif /* SERDES_TURN_DEG_H_ */
