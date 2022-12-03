/* Unit Test for serdes_turn_deg.

  Copyright © 2022 OOTA, Masato

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
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serdes_turn_deg.h"

#define ERROR(error_msg, ...) \
    do { \
        printf("Error in %s() (%s:%u): " error_msg ": ", __func__, __FILE__, __LINE__); \
        ERR_PARAM(__VA_ARGS__); \
        fputc('\n', stdout); \
    } while (0)

#define INDEX_SEQ2 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1
#define VA_ARGS_SIZE_IMPL_2(ARG11, ARG12, ARG21, ARG22, ARG31, ARG32, ARG41, ARG42, ARG51, ARG52, ARG61, ARG62, N, ...) N
#define VA_ARGS_SIZE_IMPL_1(...) VA_ARGS_SIZE_IMPL_2(__VA_ARGS__)
#define VA_ARGS_SIZE(...) VA_ARGS_SIZE_IMPL_1(__VA_ARGS__, INDEX_SEQ2)
#define MACRO_CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) MACRO_CONCAT_IMPL(x, y)
#define ERR_PARAM(...)  MACRO_CONCAT(ERR_PARAM_, VA_ARGS_SIZE(__VA_ARGS__)) (__VA_ARGS__)

#define ERR_PARAM_1(msg1, var1) printf(msg1, var1)
#define ERR_PARAM_2(msg1, var1, msg2, var2) \
    ERR_PARAM_1(msg1, var1); \
    printf(", " msg2, var2)
#define ERR_PARAM_3(msg1, var1, msg2, var2, msg3, var3) \
    ERR_PARAM_2(msg1, var1, msg2, var2); \
    printf(", " msg3, var3)
#define ERR_PARAM_4(msg1, var1, msg2, var2, msg3, var3, msg4, var4) \
    ERR_PARAM_3(msg1, var1, msg2, var2, msg3, var3); \
    printf(", " msg4, var4)
#define ERR_PARAM_5(msg1, var1, msg2, var2, msg3, var3, msg4, var4, msg5, var5) \
    ERR_PARAM_4(msg1, var1, msg2, var2, msg3, var3, msg4, var4); \
    printf(", " msg5, var5)
#define ERR_PARAM_6(msg1, var1, msg2, var2, msg3, var3, msg4, var4, msg5, var5, msg6, var6) \
    ERR_PARAM_5(msg1, var1, msg2, var2, msg3, var3, msg4, var4, msg5, var5); \
    printf(", " msg6, var6)


int test_recoverable_serialize_turn_to_deg(const unsigned int bit_width)
{
    char buf[SERDES_TURN_DEG_BUF_SIZE];
    unsigned int i;
    for (i = 0; i < 1u << bit_width; i++) {
        double deg;
        unsigned int recovered;
        const char *end_ptr = serialize_turn_to_deg(buf, i, bit_width);
        if (end_ptr - buf >= SERDES_TURN_DEG_BUF_SIZE) {
            ERROR("Buffer overflow?",
                  "bit_width: %u", bit_width,
                  "turn: %u", i,
                  "serialized deg: %s", buf);
            return 0;
        }
        if (*end_ptr != '\0') {
            ERROR("Invalid end pointer",
                  "bit_width: %u", bit_width,
                  "turn: %u", i,
                  "serialized deg: %s", buf);
            return 0;
        }
        deg = atof(buf);
        recovered = conv_deg_to_turn(deg, bit_width);
        if (i != recovered) {
            ERROR("Converted turn mismatch",
                  "bit_width: %u", bit_width,
                  "turn: %u", i,
                  "serialized deg: %s", buf,
                  "recovered turn: %u", recovered);
            return 0;
        }
        recovered = deserialize_turn_from_deg(buf, bit_width, NULL);
        if (i != recovered) {
            ERROR("Deserialized turn mismatch",
                  "bit_width: %u", bit_width,
                  "turn: %u", i,
                  "serialized deg: %s", buf,
                  "recovered turn: %u", recovered);
            return 0;
        }
    }
    return 1;
}

int test_recoverable_serialize_turn_to_deg_p(const unsigned int bit_width, const int precision)
{
    char buf[SERDES_TURN_DEG_BUF_SIZE];
    unsigned int i;
    for (i = 0; i < 1u << bit_width; i++) {
        double deg;
        unsigned int recovered;
        const char *end_ptr = serialize_turn_to_deg_p(buf, i, bit_width, precision);
        const char *p = buf;
        int decimal_place = -1;
        for (; *p != '\0'; p++) {
            if (*p == '.') {
                decimal_place = 0;
            } else if (decimal_place >= 0) {
                decimal_place++;
            }
        }
        if (precision > 0 && precision > decimal_place) {
            ERROR("Result precision mismatch",
                  "bit_width: %u", bit_width,
                  "precision: %d", precision,
                  "turn: %u", i,
                  "serialized deg: %s", buf);
            return 0;
        }
        if (*end_ptr != '\0') {
            ERROR("Invalid end pointer",
                  "bit_width: %u", bit_width,
                  "precision: %d", precision,
                  "turn: %u", i,
                  "serialized deg: %s", buf);
            return 0;
        }
        deg = atof(buf);
        recovered = conv_deg_to_turn(deg, bit_width);
        if (i != recovered) {
            ERROR("Converted turn mismatch",
                  "bit_width: %u", bit_width,
                  "precision: %d", precision,
                  "turn: %u", i,
                  "serialized deg: %s", buf,
                  "recovered turn: %u", recovered);
            return 0;
        }
        recovered = deserialize_turn_from_deg(buf, bit_width, NULL);
        if (i != recovered) {
            ERROR("Deserialized turn mismatch",
                  "bit_width: %u", bit_width,
                  "precision: %d", precision,
                  "turn: %u", i,
                  "serialized deg: %s", buf,
                  "recovered turn: %u", recovered);
            return 0;
        }
    }
    return 1;
}

int test_recoverable_serialize_turn_to_deg_ps(const unsigned int bit_width, const int precision)
{
    char buf[SERDES_TURN_DEG_BUF_SIZE];
    char buf2[SERDES_TURN_DEG_BUF_SIZE];
    unsigned int i;
    for (i = 0; i < 1u << bit_width; i++) {
        double deg;
        unsigned int recovered;
        const char *end_ptr = serialize_turn_to_deg_ps(buf, i, bit_width, precision);
        const char *end_ptr2 = serialize_turn_to_deg_p(buf2, i, bit_width, precision);
        const char *obs = buf;
        const char *exp = buf2;
        for (; obs < end_ptr && exp < end_ptr2; obs++, exp++) {
            if (*obs != *exp) {
                ERROR("Higher digit mismatch",
                      "bit_width: %u", bit_width,
                      "precision: %d", precision,
                      "turn: %u", i,
                      "serialized deg: %s", buf,
                      "serialized deg without suppress: %s", buf2);
                return 0;
            }
        }
        if (exp == end_ptr2 && obs != end_ptr) {
            ERROR("Longer than no suppression",
                  "bit_width: %u", bit_width,
                  "precision: %d", precision,
                  "turn: %u", i,
                  "serialized deg: %s", buf,
                  "serialized deg without suppress: %s", buf2);
            return 0;
        }
        if (exp != end_ptr2) {
            if (*exp == '.') {
                exp++;
                if (exp == end_ptr2) {
                    ERROR("Invalid expected value???",
                          "bit_width: %u", bit_width,
                          "precision: %d", precision,
                          "turn: %u", i,
                          "serialized deg: %s", buf,
                          "serialized deg without suppress: %s", buf2);
                    return 0;
                }
            }
            for (; exp != end_ptr2; exp++) {
                if (*exp != '0') {
                    ERROR("Suppression error",
                          "bit_width: %u", bit_width,
                          "precision: %d", precision,
                          "turn: %u", i,
                          "serialized deg: %s", buf,
                          "serialized deg without suppress: %s", buf2);
                    return 0;
                 }
            }
        }
        if (*end_ptr != '\0') {
            ERROR("Invalid end pointer",
                  "bit_width: %u", bit_width,
                  "precision: %d", precision,
                  "turn: %u", i,
                  "serialized deg: %s", buf);
            return 0;
        }
        deg = atof(buf);
        recovered = conv_deg_to_turn(deg, bit_width);
        if (i != recovered) {
            ERROR("Converted turn mismatch",
                  "bit_width: %u", bit_width,
                  "precision: %d", precision,
                  "turn: %u", i,
                  "serialized deg: %s", buf,
                  "recovered turn: %u", recovered);
            return 0;
        }
        recovered = deserialize_turn_from_deg(buf, bit_width, NULL);
        if (i != recovered) {
            ERROR("Deserialized turn mismatch",
                  "bit_width: %u", bit_width,
                  "precision: %d", precision,
                  "turn: %u", i,
                  "serialized deg: %s", buf,
                  "recovered turn: %u", recovered);
            return 0;
        }
    }
    return 1;
}


struct TestSerializeDegTable {
    unsigned int bit_width;
    unsigned int turn;
    int precision;
    char *result;
};

struct TestSerializeDegTable test_for_serialize_turn_to_deg[] = {
    { 3, 0, 0, "0" },
    { 3, 1, 0, "50" },
    { 3, 2, 0, "100" },
    { 3, 3, 0, "140" },
    { 4, 1, 1, "20" },
    { 4, 2, 1, "50" },
    { 4, 3, 1, "70" },
    { 4, 4, 1, "100" },
};

struct TestSerializeDegTable test_for_serialize_turn_to_deg_p[] = {
    { 3, 1, 0, "45" },
    { 3, 2, 0, "90" },
    { 3, 3, 0, "135" },
    { 4, 1, 1, "22.5" },
    { 4, 2, 1, "45.0" },
    { 4, 3, 1, "67.5" },
    { 4, 4, 1, "90.0" },
    { 4, 0, -2, "0" },
    { 4, 1, -2, "20" },
    { 4, 2, -2, "50" },
};

struct TestSerializeDegTable test_for_serialize_turn_to_deg_ps[] = {
    { 3, 1, 0, "45" },
    { 3, 2, 0, "90" },
    { 3, 3, 0, "135" },
    { 4, 1, 1, "22.5" },
    { 4, 2, 1, "45" },
    { 4, 3, 1, "67.5" },
    { 4, 4, 1, "90" },
    { 4, 0, -2, "0" },
    { 4, 1, -2, "20" },
    { 4, 2, -2, "50" },
};

int test_serialize_turn_to_deg(const struct TestSerializeDegTable *table, size_t n)
{
    size_t i;
    char buf[SERDES_TURN_DEG_BUF_SIZE];
    for (i = 0; i < n; i++) {
        const char *end_ptr = serialize_turn_to_deg(buf, table[i].turn, table[i].bit_width);
        if (strcmp(buf, table[i].result) != 0) {
            ERROR("Observed value is mismatched",
                  "bit_width: %u", table[i].bit_width,
                  "turn: %u", table[i].turn,
                  "expected deg: %s", table[i].result,
                  "observed deg: %s", buf);
            return 0;
        }
        if (*end_ptr != '\0') {
            ERROR("Invalid end pointer",
                  "bit_width: %u", table[i].bit_width,
                  "turn: %u", table[i].turn,
                  "result: %s", buf);
            return 0;
        }
    }
    return 1;
}

int test_serialize_turn_to_deg_p(const struct TestSerializeDegTable *table, size_t n)
{
    size_t i;
    char buf[SERDES_TURN_DEG_BUF_SIZE];
    for (i = 0; i < n; i++) {
        const char *end_ptr = serialize_turn_to_deg_p(buf, table[i].turn, table[i].bit_width, table[i].precision);
        if (strcmp(buf, table[i].result) != 0) {
            ERROR("Observed value is mismatched",
                  "bit_width: %u", table[i].bit_width,
                  "precision: %d", table[i].precision,
                  "turn: %u", table[i].turn,
                  "expected deg: %s", table[i].result,
                  "observed deg: %s", buf);
            return 0;
        }
        if (*end_ptr != '\0') {
            ERROR("Invalid end pointer",
                  "bit_width: %u", table[i].bit_width,
                  "precision: %d", table[i].precision,
                  "turn: %u", table[i].turn,
                  "result: %s", buf);
            return 0;
        }
    }
    return 1;
}


int test_serialize_turn_to_deg_ps(const struct TestSerializeDegTable *table, size_t n)
{
    size_t i;
    char buf[SERDES_TURN_DEG_BUF_SIZE];
    for (i = 0; i < n; i++) {
        const char *end_ptr = serialize_turn_to_deg_ps(buf, table[i].turn, table[i].bit_width, table[i].precision);
        if (strcmp(buf, table[i].result) != 0) {
            ERROR("Observed value is mismatched",
                  "bit_width: %u", table[i].bit_width,
                  "precision: %d", table[i].precision,
                  "turn: %u", table[i].turn,
                  "expected deg: %s", table[i].result,
                  "observed deg: %s", buf);
            return 0;
        }
        if (*end_ptr != '\0') {
            ERROR("Invalid end pointer",
                  "bit_width: %u", table[i].bit_width,
                  "precision: %d", table[i].precision,
                  "turn: %u", table[i].turn,
                  "result: %s", buf);
            return 0;
        }
    }
    return 1;
}


struct TestDeserializeDegTable {
    char *input;
    unsigned int bit_width;
    unsigned int turn;
    int end_index;
};


struct TestDeserializeDegTable test_for_deserialize_turn_from_deg[] = {
    { "89.99999", 1, 0, 8 },
    { "90", 1, 1, 2 },
    { "123", 1, 1, 3 },
    { "269.99999", 1, 1, 9 },
    { "270", 1, 0, 3 },
    { "350", 1, 0, 3 },
    { ".12345", 1, 0, 6 },
    { "1.", 1, 0, 2 },
    { "999.999", 1, 0, 7 },
    { "0000.000", 1, 0, 3 },
    { "1.234567890", 1, 0, 11 },
    { "180.1.1", 1, 1, 5 },
    { "+180.0", 2, 0, 0 },
    { "-180.0", 2, 0, 0 },
    { "    180.0", 2, 2, 9 },
    { "180.0    ", 2, 2, 5 },
    { "270.02b  ", 2, 3, 6 },
};

int test_deserialize_turn_from_deg(const struct TestDeserializeDegTable *table, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        const char *endptr;
        int ptr_diff;
        const unsigned int turn = deserialize_turn_from_deg(table[i].input, table[i].bit_width, &endptr);
        ptr_diff = endptr - table[i].input;
        if (turn != table[i].turn) {
            ERROR("Observed turn is mismatched",
                  "input text: %s", table[i].input,
                  "bit_width: %u", table[i].bit_width,
                  "expected turn: %u", table[i].turn,
                  "observed turn: %u", turn,
                  "expected end index: %d", table[i].end_index,
                  "observed end index: %d", ptr_diff);
            return 0;
        }
        if (ptr_diff != table[i].end_index) {
            ERROR("Observed end index is mismatched",
                  "input text: %s", table[i].input,
                  "bit_width: %u", table[i].bit_width,
                  "expected turn: %u", table[i].turn,
                  "observed turn: %u", turn,
                  "expected end index: %d", table[i].end_index,
                  "observed end index: %d", ptr_diff);
            return 0;
        }
    }
    return 1;
}


int test_deg_is_just_integer()
{
    char buf[SERDES_TURN_DEG_BUF_SIZE];
    char buf2[4] = "0";
    const unsigned int BIT_WIDTH = 9;
    unsigned int i = 0;
    unsigned int turn;
    for (turn = 0; turn < (1u << BIT_WIDTH); turn++) {
        serialize_turn_to_deg(buf, turn, BIT_WIDTH);
        if (strcmp(buf, buf2) == 0) {
            i++;
            sprintf(buf2, "%u", i);
        }
    }
    if (i != 360) {
        ERROR("Too low resolution",
              "bit_width: %u", BIT_WIDTH,
              "last degree: %u (expected 360)", i);
        return 0;
    }
    return 1;
}



#define NUM_OF(a) (sizeof(a)/sizeof(a[0]))

int main()
{
    unsigned int bit_width;
    int precision;

    fputs("Testing: Serialize and then Deserialize: serialize_turn_to_deg()\n", stdout);
    for (bit_width = SERDES_TURN_DEG_MIN_BIT_WIDTH; bit_width <= SERDES_TURN_DEG_MAX_BIT_WIDTH; bit_width++) {
        if (!test_recoverable_serialize_turn_to_deg(bit_width)) {
            return 1;
        }
    }

    fputs("Testing: Serialize and then Deserialize: serialize_turn_to_deg_p()\n", stdout);
    for (bit_width = SERDES_TURN_DEG_MIN_BIT_WIDTH; bit_width <= SERDES_TURN_DEG_MAX_BIT_WIDTH; bit_width++) {
        for (precision = SERDES_TURN_DEG_MIN_PRECISION; precision <= SERDES_TURN_DEG_MAX_PRECISION; precision++) {
            if (!test_recoverable_serialize_turn_to_deg_p(bit_width, precision)) {
                return 1;
            }
        }
    }

    fputs("Testing: Serialize and then Deserialize: serialize_turn_to_deg_ps()\n", stdout);
    for (bit_width = SERDES_TURN_DEG_MIN_BIT_WIDTH; bit_width <= SERDES_TURN_DEG_MAX_BIT_WIDTH; bit_width++) {
        for (precision = SERDES_TURN_DEG_MIN_PRECISION; precision <= SERDES_TURN_DEG_MAX_PRECISION; precision++) {
            if (!test_recoverable_serialize_turn_to_deg_ps(bit_width, precision)) {
                return 1;
            }
        }
    }

    fputs("Testing: Certain Patterns: serialize_turn_to_deg()\n", stdout);
    if (!test_serialize_turn_to_deg(test_for_serialize_turn_to_deg, NUM_OF(test_for_serialize_turn_to_deg))) {
        return 1;
    }

    fputs("Testing: Certain Patterns: serialize_turn_to_deg_p()\n", stdout);
    if (!test_serialize_turn_to_deg_p(test_for_serialize_turn_to_deg_p, NUM_OF(test_for_serialize_turn_to_deg_p))) {
        return 1;
    }

    fputs("Testing: Certain Patterns: serialize_turn_to_deg_ps()\n", stdout);
    if (!test_serialize_turn_to_deg_ps(test_for_serialize_turn_to_deg_ps, NUM_OF(test_for_serialize_turn_to_deg_ps))) {
        return 1;
    }

    fputs("Testing: Certain Patterns: deserialize_turn_from_deg()\n", stdout);
    if (!test_deserialize_turn_from_deg(test_for_deserialize_turn_from_deg, NUM_OF(test_for_deserialize_turn_from_deg))) {
        return 1;
    }

    fputs("Testing: Resolution\n", stdout);
    if (!test_deg_is_just_integer()) {
        return 1;
    }

    return 0;
}
