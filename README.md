
<!--
This file is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This file is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this file.  If not, see <http://www.gnu.org/licenses/>.

Copyright © 2022 OOTA, Masato
-->

# Serialize/deserialize an angle @ turn expressed by a fixed point number to/from @ degree.
## Introduction
This is a niche library to serialize/deserialize an angle @ turn to/from @ degree.

Commonly, in the embedded applications and the hardware logics that handle an angle, the engineers use 2π rad / 2<sup>n</sup> as unit of the angle; 2π rad / 2<sup>n</sup> means that 2<sup>n</sup> represent a full circle so it makes easy to handle wrap around. They consider the number as a fixed point number, meaning 2<sup>n</sup> as 1.0, so this library calls the unit 'turn'.

When you would serialize an angle @ turn for data exchange, you can multiply it by 2π and output it as a double value, it's a proper solution and you have no need to use this library, but in case your clients look in a data file and say "The unit should be degree" and "It should be 20 instead of 20.00061", this library comes on stage.

## Source Codes
There are [one header file](@ref source/serdes_turn_deg.h) and [one source file](@ref source/serdes_turn_deg.c) in @ref source/.

The target language is C89.

## Algorithm
This library uses a variation of (FPP)<sub>2</sub> in [dragon4](https://dl.acm.org/doi/10.1145/93548.93559) to serialize an angle. Basically, it chooses a number that is the shortest digits of radix 10 in the range of the true value ± LSB/2. The LSB @ turn is larger than the LSB of the value multiplied by 2π, so the required number of the digit place is smaller.

Note that it means that when a bit-width of a value is a quite small, such as 8, LSB is larger than you might expect and the result might be against your intuition. However, I guess you and your clients must be experts in your application and numeric calculation if you may determine a sufficient bit-width to be such a small number.

## Three Functions to Serialize
There are three functions to serialize. The basic function is [serialize_turn_to_deg()](@ref serialize_turn_to_deg()) and I recommend it in most cases. When you want to specify the number of the digit, you can use [serialize_turn_to_deg_p()](@ref serialize_turn_to_deg_p()). To suppress a series of the lowest digits zero, [serialize_turn_to_deg_ps()](@ref serialize_turn_to_deg_ps()).

A example of bit_width = 16
| angle | result#1 | result#2 | result#3 |
|-------|----------|----------|----------|
|   0   |   0      |   0.0    |   0      |
|   1   |   0.005  |   0.005  |   0.005  |
|   2   |   0.01   |   0.01   |   0.01   |
|   3   |   0.016  |   0.016  |   0.016  |
|   4   |   0.02   |   0.02   |   0.02   |

1. [serialize_turn_to_deg](@ref serialize_turn_to_deg())(result#1, angle, bit_width)
2. [serialize_turn_to_deg_p](@ref serialize_turn_to_deg_p())(result#2, angle, bit_width, 1)
3. [serialize_turn_to_deg_ps](@ref serialize_turn_to_deg_ps())(result#3, angle, bit_width, 1)

2 and 3 specify 1 as the precision, but the results can be longer than it since the precision is not sufficient to recover the same angle.


A example of bit_width = 4
| angle | result#1 | result#2 | result#3 |
|-------|----------|----------|----------|
|   0   |     0    |    0.0   |    0     |
|   1   |    20    |   22.5   |   22.5   |
|   2   |    50    |   45.0   |   45     |
|   3   |    70    |   67.5   |   67.5   |
|   4   |   100    |   90.0   |   90     |

Why is result#1 100 when the true value is 90? Because 90 is within 100 ± LSB/2 and the algorithm try to determine the value from the higher digit place.

4 is too small as a bit-width, isn't it? If your clients only want an integer for any angles, even for [serialize_turn_to_deg()](@ref serialize_turn_to_deg()), 9 is an enough bit-width. Anyway, please remember that the purpose of this library is to serialize and deserialize for the data exchange. If you want a true value, you need only to multiply a turn by 2π rad or 360 degrees.

## Documents
Use doxygen to generate the documents. I tested the generation in doxygen 1.9.4.

```
% cd serdes_turn
% doxygen serdes_turn.doxygen
# Open serdes_turn/html/index.html with your HTML browser.
```

## Unit Test
The unit test code is written by C99, since the code needs variadic macros and \_\_func\_\_ macro.

```
% cd serdes_turn
% make
```

If you want to use meson:
```
% cd serdes_turn
% meson setup build
% cd build
% meson test
```

## Install
Use meson:
```
% cd serdes_turn
% meson setup build
% cd build
% meson install
```
It generates and installs a library, a header, and some manual files.

## License
See copyright file for the copyright notice and the license details.

This program is published under GPL-3.0-or-later:

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
