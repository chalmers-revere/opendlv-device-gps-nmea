/*
 * Copyright (C) 2018  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NMEADECODERCONSTANTS_HPP
#define NMEADECODERCONSTANTS_HPP

#include <cstdint>

enum NMEADecoderConstants {
    UNKNOWN     = 0,
    BUFFER_SIZE = 2048,
    HEADER_SIZE = 6,    /*$--XYZ*/
};

#endif
